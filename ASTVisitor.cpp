#include "ASTVisitor.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"

bool ASTVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl *Declaration) {
    if (Declaration->hasDefinition()) {
        const clang::RawComment *RC = Context->getRawCommentForDeclNoCache(Declaration);
        if (RC && RC->getKind() == clang::RawComment::CommentKind::RCK_JavaDoc) {
            clang::StringRef CommentText = RC->getRawText(Context->getSourceManager());
            if (CommentText.contains("@component")) {
                std::string ClassName = Declaration->getQualifiedNameAsString();
                Component component = componentParser.parse(Declaration, Context);
                Components.push_back(std::move(component));
            }
        }
    }
    return true;
}

//ASTVisitor::ASTVisitor(clang::ASTContext *Context) : Context(Context) {}
ASTVisitor::ASTVisitor(clang::ASTContext *Context, ErrorCollector& errorCollector)
    : Context(Context), componentParser(errorCollector) {}
    
std::vector<Component> ASTVisitor::GetComponents() const {
    return Components;
}