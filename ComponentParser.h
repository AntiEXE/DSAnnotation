#pragma once
#include "ReferenceParser.h"
#include "PropertyParser.h"
#include "Component.h"
#include "ErrorCollector.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/RawCommentList.h"
#include "clang/AST/Decl.h"
#include "clang/AST/ASTContext.h"
#include <fstream>

using json = nlohmann::json;
using namespace clang;

class ComponentParser {
public:
    ComponentParser(ErrorCollector& errorCollector) : errorCollector(errorCollector) {};
    Component parse(const clang::CXXRecordDecl* Declaration, clang::ASTContext* Context);

private:
    void parseReferences(Component& component, const clang::CXXRecordDecl* Declaration, clang::ASTContext* Context);
    void parseProperties(Component& component, const clang::RawComment* RC, clang::ASTContext* Context);
    void parseExternalProperties(Component& component, const std::string& filePath, clang::ASTContext* Context);
    ReferenceParser referenceParser;
    PropertyParser propertyParser;
    ErrorCollector& errorCollector;
};
