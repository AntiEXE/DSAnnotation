#include "DSAnnotation/Parsing/ComponentParser.h"

#include <utility>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/RawCommentList.h"
#include "clang/AST/Type.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Path.h"
#include "nlohmann/json.hpp"

namespace dsannotation::parsing {

// Helper function to convert Clang SourceLocation to our SourceLocationInfo
static support::SourceLocationInfo convertSourceLocation(clang::SourceLocation loc,
                                                         const clang::SourceManager& sourceManager) {
    if (loc.isInvalid()) {
        return {};
    }
    
    auto filename = sourceManager.getFilename(loc);
    auto line = sourceManager.getSpellingLineNumber(loc);
    auto column = sourceManager.getSpellingColumnNumber(loc);
    
    return support::SourceLocationInfo(filename.str(), line, column);
}

ComponentParser::ComponentParser(const IPropertyParser& propertyParser,
                                 const IReferenceParser& referenceParser,
                                 const support::ISyntaxChecker& syntaxChecker,
                                 const support::IFileSystem& fileSystem,
                                 core::ErrorCollector& errorCollector,
                                 const config::ParserConfig& config)
    : propertyParser_(propertyParser),
      referenceParser_(referenceParser),
      syntaxChecker_(syntaxChecker),
      fileSystem_(fileSystem),
      errorCollector_(errorCollector),
      config_(config) {}

core::Component ComponentParser::parse(const clang::CXXRecordDecl& declaration,
                                       clang::ASTContext& context) const {
    core::Component component(declaration.getQualifiedNameAsString());

    for (const auto& base : declaration.bases()) {
        if (const auto* baseDecl = base.getType()->getAsCXXRecordDecl()) {
            component.addInterface(baseDecl->getQualifiedNameAsString());
        }
    }

    const auto* comment = context.getRawCommentForDeclNoCache(&declaration);
    if (comment) {
        auto locationInfo = convertSourceLocation(comment->getBeginLoc(), context.getSourceManager());
        if (!syntaxChecker_.checkBalancedBraces(comment->getRawText(context.getSourceManager()).str(),
                                                errorCollector_,
                                                component.className(),
                                                locationInfo)) {
            // continue parsing but errors will be recorded
        }

        parseComponentAttributes(component, *comment, context);
        parseProperties(component, *comment, declaration, context);
    }

    parseReferences(component, declaration, context);

    return component;
}

void ComponentParser::parseComponentAttributes(core::Component& component,
                                               const clang::RawComment& comment,
                                               clang::ASTContext& context) const {
    auto text = comment.getRawText(context.getSourceManager());
    auto componentStart = text.find("@component");
    if (componentStart == llvm::StringRef::npos) {
        return;
    }

    auto attrStart = text.find('{', componentStart);
    auto attrEnd = text.find('}', attrStart);
    if (attrStart == llvm::StringRef::npos || attrEnd == llvm::StringRef::npos || attrStart >= attrEnd) {
        return;
    }

    auto attributesString = text.substr(attrStart + 1, attrEnd - attrStart - 1);
    auto attributes = propertyParser_.parse(attributesString.str());
    if (!attributes.empty()) {
        component.setAttributes(std::move(attributes));
    }
}

void ComponentParser::parseProperties(core::Component& component,
                                      const clang::RawComment& comment,
                                      const clang::CXXRecordDecl& declaration,
                                      clang::ASTContext& context) const {
    auto commentText = comment.getRawText(context.getSourceManager());

    const auto propertiesPos = commentText.find("@properties");
    if (propertiesPos != llvm::StringRef::npos) {
        auto jsonStart = commentText.find('{', propertiesPos);
        auto jsonEnd = commentText.rfind('}');
        if (jsonStart != llvm::StringRef::npos && jsonEnd != llvm::StringRef::npos && jsonStart < jsonEnd) {
            auto jsonString = commentText.substr(jsonStart, jsonEnd - jsonStart + 1).str();
            auto parsed = nlohmann::json::parse(jsonString, nullptr, false);
            if (!parsed.is_discarded()) {
                component.setProperties(std::move(parsed));
            } else {
                auto locationInfo = convertSourceLocation(comment.getBeginLoc(), context.getSourceManager());
                errorCollector_.addError("Invalid JSON in @properties for " + component.className(),
                                         locationInfo,
                                         core::ErrorSeverity::Error,
                                         core::ErrorCategory::Property);
            }
        }
        return;
    }

    const auto propertyPos = commentText.find("@property");
    if (propertyPos != llvm::StringRef::npos) {
        auto pathStart = commentText.find('{', propertyPos);
        auto pathEnd = commentText.find('}', pathStart);
        if (pathStart != llvm::StringRef::npos && pathEnd != llvm::StringRef::npos && pathStart < pathEnd) {
            auto filePath = commentText.substr(pathStart + 1, pathEnd - pathStart - 1).str();
            parseExternalProperties(component, filePath, declaration, context);
        }
    }
}

void ComponentParser::parseReferences(core::Component& component,
                                      const clang::CXXRecordDecl& declaration,
                                      clang::ASTContext& context) const {
    for (const auto* constructor : declaration.ctors()) {
        if (!constructor) {
            continue;
        }
        const auto* comment = context.getRawCommentForDeclNoCache(constructor);
        if (!comment) {
            continue;
        }
        auto commentText = comment->getRawText(context.getSourceManager());
        if (!commentText.contains("@reference")) {
            continue;
        }

        for (const auto* param : constructor->parameters()) {
            if (!param) {
                continue;
            }
            auto [qualified, simplified] = extractInterfaceNames(*param, context);
            auto reference = referenceParser_.parse(commentText.str(), simplified, qualified);
            component.addReference(std::move(reference));
        }
    }
}

void ComponentParser::parseExternalProperties(core::Component& component,
                                              const std::string& filePath,
                                              const clang::CXXRecordDecl& declaration,
                                              clang::ASTContext& context) const {
    const auto& sourceManager = context.getSourceManager();
    auto presumed = sourceManager.getPresumedLoc(declaration.getLocation());
    std::string resolvedPath = filePath;

    if (presumed.isValid()) {
        llvm::SmallString<256> basePath(presumed.getFilename());
        llvm::sys::path::remove_filename(basePath);
        llvm::sys::path::append(basePath, filePath);
        resolvedPath = basePath.str().str();
    }

    auto jsonContent = fileSystem_.readJsonFile(resolvedPath);
    if (!jsonContent) {
        auto locationInfo = convertSourceLocation(declaration.getLocation(), context.getSourceManager());
        errorCollector_.addError("Unable to read properties file: " + resolvedPath,
                                 locationInfo,
                                 core::ErrorSeverity::Error,
                                 core::ErrorCategory::Property);
        return;
    }

    component.setProperties(std::move(*jsonContent));
}

std::pair<std::string, std::string> ComponentParser::extractInterfaceNames(const clang::ParmVarDecl& param,
                                                                           clang::ASTContext& context) const {
    clang::QualType spelled = param.getType().getNonReferenceType();
    clang::QualType target = spelled;

    if (const auto* specialization = target->getAs<clang::TemplateSpecializationType>()) {
        if (const auto* templateDecl = specialization->getTemplateName().getAsTemplateDecl()) {
            const std::string qualifiedName = templateDecl->getQualifiedNameAsString();
            if (qualifiedName == "std::shared_ptr" || qualifiedName == "std::unique_ptr") {
                const auto& args = specialization->template_arguments();
                if (!args.empty() && args[0].getKind() == clang::TemplateArgument::Type) {
                    target = args[0].getAsType().getNonReferenceType();
                }
            }
        }
    }

    clang::QualType canonical = context.getCanonicalType(target).getUnqualifiedType();

    clang::PrintingPolicy fqPolicy(context.getLangOpts());
    fqPolicy.adjustForCPlusPlus();
    fqPolicy.FullyQualifiedName = true;
    std::string fullyQualifiedName = canonical.getAsString(fqPolicy);

    clang::PrintingPolicy displayPolicy(context.getLangOpts());
    displayPolicy.adjustForCPlusPlus();
    displayPolicy.FullyQualifiedName = false;
    std::string displayName = target.getAsString(displayPolicy);

    std::string simplified = displayName;
    if (auto lt = simplified.rfind('<'); lt != std::string::npos) {
        simplified = simplified.substr(lt + 1);
        if (!simplified.empty() && simplified.back() == '>') {
            simplified.pop_back();
        }
    }

    llvm::StringRef simplifiedRef(simplified);
    simplifiedRef = simplifiedRef.trim();
    if (auto pos = simplifiedRef.rfind("::"); pos != llvm::StringRef::npos) {
        simplified = simplifiedRef.substr(pos + 2).str();
    } else {
        simplified = simplifiedRef.str();
    }

    return {fullyQualifiedName, simplified};
}

} // namespace dsannotation::parsing
