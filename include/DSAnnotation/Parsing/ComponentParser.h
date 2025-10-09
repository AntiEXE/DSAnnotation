#pragma once

#include <string>

#include "DSAnnotation/Config/ParserConfig.h"
#include "DSAnnotation/Core/Component.h"
#include "DSAnnotation/Core/ErrorCollector.h"
#include "DSAnnotation/Parsing/IComponentParser.h"
#include "DSAnnotation/Parsing/IPropertyParser.h"
#include "DSAnnotation/Parsing/IReferenceParser.h"
#include "DSAnnotation/Support/IFileSystem.h"
#include "DSAnnotation/Support/ISyntaxChecker.h"

namespace clang {
class ASTContext;
class CXXRecordDecl;
class RawComment;
class ParmVarDecl;
}

namespace dsannotation::parsing {

class ComponentParser final : public IComponentParser {
public:
    ComponentParser(const IPropertyParser& propertyParser,
                    const IReferenceParser& referenceParser,
                    const support::ISyntaxChecker& syntaxChecker,
                    const support::IFileSystem& fileSystem,
                    core::ErrorCollector& errorCollector,
                    const config::ParserConfig& config);

    core::Component parse(const clang::CXXRecordDecl& declaration,
                          clang::ASTContext& context) const override;

private:
    void parseComponentAttributes(core::Component& component,
                                  const clang::RawComment& comment,
                                  clang::ASTContext& context) const;

    void parseProperties(core::Component& component,
                         const clang::RawComment& comment,
                         const clang::CXXRecordDecl& declaration,
                         clang::ASTContext& context) const;

    void parseReferences(core::Component& component,
                         const clang::CXXRecordDecl& declaration,
                         clang::ASTContext& context) const;

    void parseExternalProperties(core::Component& component,
                                 const std::string& filePath,
                                 const clang::CXXRecordDecl& declaration,
                                 clang::ASTContext& context) const;

    std::pair<std::string, std::string> extractInterfaceNames(const clang::ParmVarDecl& param,
                                                              clang::ASTContext& context) const;

    const IPropertyParser& propertyParser_;
    const IReferenceParser& referenceParser_;
    const support::ISyntaxChecker& syntaxChecker_;
    const support::IFileSystem& fileSystem_;
    core::ErrorCollector& errorCollector_;
    const config::ParserConfig& config_;
};

} // namespace dsannotation::parsing
