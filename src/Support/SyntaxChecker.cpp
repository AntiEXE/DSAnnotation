#include "DSAnnotation/Support/SyntaxChecker.h"

#include "DSAnnotation/Support/SourceLocationInfo.h"

#include <stack>

namespace dsannotation::support {

bool SyntaxChecker::checkBalancedBraces(const std::string& text,
                                        core::ErrorCollector& errorCollector,
                                        std::string_view contextLabel,
                                        const SourceLocationInfo& location) const {
    std::stack<char> braces;
    for (char ch : text) {
        if (ch == '{') {
            braces.push('{');
        } else if (ch == '}') {
            if (braces.empty()) {
                if (location.isValid()) {
                    // Create a simple SourceLocation from our info for ErrorCollector
                    std::string errorMsg = "Unmatched closing brace in " + std::string(contextLabel) + 
                                          " at " + location.filename + ":" + std::to_string(location.line) + 
                                          ":" + std::to_string(location.column);
                    errorCollector.addError(errorMsg,
                                           core::ErrorSeverity::Error,
                                           core::ErrorCategory::General);
                } else {
                    errorCollector.addError("Unmatched closing brace in " + std::string(contextLabel),
                                           core::ErrorSeverity::Error,
                                           core::ErrorCategory::General);
                }
                return false;
            }
            braces.pop();
        }
    }

    if (!braces.empty()) {
        if (location.isValid()) {
            std::string errorMsg = "Missing closing brace in " + std::string(contextLabel) + 
                                  " at " + location.filename + ":" + std::to_string(location.line) + 
                                  ":" + std::to_string(location.column);
            errorCollector.addError(errorMsg,
                                   core::ErrorSeverity::Error,
                                   core::ErrorCategory::General);
        } else {
            errorCollector.addError("Missing closing brace in " + std::string(contextLabel),
                                   core::ErrorSeverity::Error,
                                   core::ErrorCategory::General);
        }
        return false;
    }

    return true;
}

} // namespace dsannotation::support
