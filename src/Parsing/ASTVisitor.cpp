#include "DSAnnotation/Parsing/ASTVisitor.h"

namespace dsannotation::parsing {

ASTVisitor::ASTVisitor(clang::ASTContext& context, const IComponentParser& componentParser)
    : context_(context), componentParser_(componentParser) {}

bool ASTVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl* declaration) {
    if (!declaration || !declaration->hasDefinition()) {
        return true;
    }

    const auto* rawComment = context_.getRawCommentForDeclNoCache(declaration);
    if (!rawComment) {
        return true;
    }

    auto commentText = rawComment->getRawText(context_.getSourceManager());
    if (!commentText.contains("@component")) {
        return true;
    }

    auto component = componentParser_.parse(*declaration, context_);
    components_.push_back(std::move(component));
    return true;
}

} // namespace dsannotation::parsing
