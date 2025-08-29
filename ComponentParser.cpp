#include "ComponentParser.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
// #include "Logger.h"
#include "SyntaxChecker.h"
#include <regex>

using namespace clang;

Component ComponentParser::parse(const clang::CXXRecordDecl* record, clang::ASTContext* Context) {
    Component component(record->getQualifiedNameAsString());
    
    // Extract interfaces (existing logic)
    for (const auto& base : record->bases()) {
        if (const auto* baseDecl = base.getType()->getAsCXXRecordDecl()) {
            component.addInterface(baseDecl->getQualifiedNameAsString());
        }
    }
    
    // Parse comments for annotations
    const clang::RawComment* RC = Context->getRawCommentForDeclNoCache(record);
    if (RC) {
        // Parse component-level attributes (@component {key=value})
        parseComponentAttributes(component, RC, Context);
        
        // Parse properties (@properties, @property)
        parseProperties(component, RC, Context);
        
        // Parse references (existing logic)
        parseReferences(component, record, Context);
    }
    
    return component;
}

std::vector<std::string> DeriveNameFromType(const std::string& typeName) {
    std::string extractedTypeName = typeName;
    
    // Remove 'const' if present
    size_t constPos = extractedTypeName.find("const ");
    if (constPos != std::string::npos) {
        extractedTypeName.erase(constPos, 6);
    }
    
    // Remove 'std::shared_ptr<' and '>' if present
    size_t sharedPtrPos = extractedTypeName.find("std::shared_ptr<");
    if (sharedPtrPos != std::string::npos) {
        extractedTypeName.erase(sharedPtrPos, 16);
        extractedTypeName.erase(extractedTypeName.find_last_of('>'));
    }
    
    // Remove '&' if present
    size_t refPos = extractedTypeName.find('&');
    if (refPos != std::string::npos) {
        extractedTypeName.erase(refPos);
    }
    
    // Trim any leading or trailing whitespace
    extractedTypeName = std::regex_replace(extractedTypeName, std::regex("^\\s+|\\s+$"), "");
    
    // Find the last occurrence of '::' and extract the simplified name
    size_t lastColon = extractedTypeName.rfind("::");
    std::string simplifiedName = (lastColon != std::string::npos) ? 
        extractedTypeName.substr(lastColon + 2) : extractedTypeName;
    
    return {extractedTypeName, simplifiedName};
}

void ComponentParser::parseReferences(Component& component, const clang::CXXRecordDecl* Declaration, clang::ASTContext* Context) {
    for (const auto* Constructor : Declaration->ctors()) {
        const clang::RawComment *ConstructorRC = Context->getRawCommentForDeclNoCache(Constructor);
        if (ConstructorRC) {
            clang::StringRef ConstructorCommentText = ConstructorRC->getRawText(Context->getSourceManager());
            if (ConstructorCommentText.contains("@reference")) {
                for (const auto* Param : Constructor->parameters()) {
                    clang::QualType ParamType = Param->getType();
                    std::string ParamTypeName = ParamType.getAsString();
                    std::string ParamName = Param->getNameAsString();
                    std::vector<std::string> RefNameAndInterface = DeriveNameFromType(ParamTypeName);
                    
                    // Use the clean interface name (RefNameAndInterface[1]) as the reference name
                    // and the full interface type (RefNameAndInterface[0]) as the interface
                    Reference ref = referenceParser.parse(ConstructorCommentText.str(), RefNameAndInterface[1], RefNameAndInterface[0]);
                    component.addReference(ref);
                }
            }
        }
    }
}

void ComponentParser::parseProperties(Component& component, const clang::RawComment* RC, clang::ASTContext* Context) {
    clang::StringRef CommentText = RC->getRawText(Context->getSourceManager());
    
    // Handle @properties (goes into properties section)
    size_t propertiesStart = CommentText.find("@properties");
    if (propertiesStart != clang::StringRef::npos) {
        size_t jsonStart = CommentText.find('{', propertiesStart);
        size_t jsonEnd = CommentText.rfind('}');
        if (jsonStart != clang::StringRef::npos && jsonEnd != clang::StringRef::npos && jsonStart < jsonEnd) {
            std::string jsonString = CommentText.substr(jsonStart, jsonEnd - jsonStart + 1).str();
            nlohmann::json properties = nlohmann::json::parse(jsonString, nullptr, false);
            if (!properties.is_discarded()) {
                component.setProperties(properties);
            }
        }
    }
    // Handle @property (external file - goes into properties section)
    else if (size_t propertyStart = CommentText.find("@property"); propertyStart != clang::StringRef::npos) {
        size_t pathStart = CommentText.find('{', propertyStart);
        size_t pathEnd = CommentText.find('}', pathStart);
        if (pathStart != clang::StringRef::npos && pathEnd != clang::StringRef::npos) {
            std::string filePath = CommentText.substr(pathStart + 1, pathEnd - pathStart - 1).str();
            parseExternalProperties(component, filePath, Context);
        }
    }
}

//Currently it is parsing the properties relative to pwd, which is the tool directory. It should take path from the pwd. 
void ComponentParser::parseExternalProperties(Component& component, const std::string& filePath, clang::ASTContext* Context) {
    std::ifstream file(filePath);
    if (file.is_open()) {
        nlohmann::json properties = nlohmann::json::parse(file, nullptr, false);
        if (!properties.is_discarded()) {
            component.setProperties(properties);
        } else {
            // Use ErrorCollector to report parsing error
            llvm::errs() << "Error parsing properties from file: " << filePath << "\n";
        }
    } else {
        // Use ErrorCollector to report file not found error
        llvm::errs() << "Error opening file: " << filePath << "\n";
    }
}

// NEW: Parse component attributes (separate method)
void ComponentParser::parseComponentAttributes(Component& component, const clang::RawComment* RC, clang::ASTContext* Context) {
    clang::StringRef CommentText = RC->getRawText(Context->getSourceManager());
    
    // Handle @component {key=value, key=value} - component-level attributes
    size_t componentStart = CommentText.find("@component");
    if (componentStart != clang::StringRef::npos) {
        size_t attrStart = CommentText.find('{', componentStart);
        size_t attrEnd = CommentText.find('}', attrStart);
        if (attrStart != clang::StringRef::npos && attrEnd != clang::StringRef::npos) {
            std::string attributesString = CommentText.substr(attrStart + 1, attrEnd - attrStart - 1).str();
            // Use PropertyParser to parse key=value pairs with dot notation support
            nlohmann::json attributes = propertyParser.parse(attributesString);
            if (!attributes.empty()) {
                component.setAttributes(attributes);
            }
        }
    }
}