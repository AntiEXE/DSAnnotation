#include "dsannotation/parsing/ASTVisitor.h"

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
    
    // Only add components that were successfully parsed
    // Invalid components (due to malformed annotations) will have empty class names
    if (!component.className().empty()) {
        components_.push_back(std::move(component));
    }
    // Note: Validation errors are already reported by ComponentParser
    
    return true;
}

} // namespace dsannotation::parsing
