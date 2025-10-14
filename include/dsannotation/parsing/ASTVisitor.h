#pragma once

#include <vector>

#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include "dsannotation/core/Component.h"
#include "dsannotation/parsing/IComponentParser.h"

namespace dsannotation::parsing {

class ASTVisitor : public clang::RecursiveASTVisitor<ASTVisitor> {
public:
    ASTVisitor(clang::ASTContext& context, const IComponentParser& componentParser);

    bool VisitCXXRecordDecl(clang::CXXRecordDecl* declaration);

    const core::ComponentList& components() const noexcept { return components_; }

private:
    clang::ASTContext& context_;
    const IComponentParser& componentParser_;
    core::ComponentList components_;
};

} // namespace dsannotation::parsing
