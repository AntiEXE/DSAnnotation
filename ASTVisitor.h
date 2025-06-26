#pragma once
#include "clang/AST/RecursiveASTVisitor.h"
#include "ComponentParser.h"
#include "ErrorCollector.h"
#include <vector>

using namespace clang;

class ASTVisitor : public clang::RecursiveASTVisitor<ASTVisitor> {
public:
    // explicit ASTVisitor(clang::ASTContext *Context, ErrorCollector& collector);
    explicit ASTVisitor(clang::ASTContext *Context, ErrorCollector& errorCollector);
    bool VisitCXXRecordDecl(clang::CXXRecordDecl *Declaration);
    std::vector<Component> GetComponents() const;

private:
    clang::ASTContext *Context;
    ComponentParser componentParser;
    std::vector<Component> Components;
};
