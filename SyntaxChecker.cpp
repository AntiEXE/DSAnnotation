#include "SyntaxChecker.h"

#include <stack>

bool SyntaxChecker::checkBalancedBraces(const std::string& text, ErrorCollector& errorCollector, const clang::SourceLocation& loc) {
    std::stack<char> braceStack;
    for (size_t i = 0; i < text.length(); ++i) {
        if (text[i] == '{') {
            braceStack.push('{');
        } else if (text[i] == '}') {
            if (braceStack.empty()) {
                errorCollector.addError("Unmatched closing brace", loc, ErrorSeverity::ERROR, ErrorCategory::GENERAL);
                return false;
            }
            braceStack.pop();
        }
    }
    if (!braceStack.empty()) {
        errorCollector.addError("Missing closing brace", loc, ErrorSeverity::ERROR, ErrorCategory::GENERAL);
        return false;
    }
    return true;
}

// Implement other check methods similarly
