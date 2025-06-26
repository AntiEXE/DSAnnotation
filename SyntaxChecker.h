#pragma once
#include <string>
#include "ErrorCollector.h"

class SyntaxChecker {
public:
    static bool checkBalancedBraces(const std::string& text, ErrorCollector& errorCollector, const clang::SourceLocation& loc);
    static bool checkBalancedParentheses(const std::string& text, ErrorCollector& errorCollector, const clang::SourceLocation& loc);
    static bool checkValidPropertyFormat(const std::string& text, ErrorCollector& errorCollector, const clang::SourceLocation& loc);
    // Add more check methods as needed
};
