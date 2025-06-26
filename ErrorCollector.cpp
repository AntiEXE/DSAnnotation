// ErrorCollector.cpp
#include "ErrorCollector.h"

void ErrorCollector::addError(const std::string& message, clang::SourceLocation location, 
                              ErrorSeverity severity, ErrorCategory category) {
    errors.push_back({message, location, severity, category});
}

std::string ErrorCollector::getSourceLine(const clang::SourceLocation& loc) const {
    unsigned int lineNum = SM->getSpellingLineNumber(loc);
    unsigned int colNum = SM->getSpellingColumnNumber(loc);

    bool invalid = false;
    const char *start = SM->getCharacterData(loc, &invalid);
    if (invalid) return "";
    const char *lineStart = start - (colNum - 1);
    const char *lineEnd = strchr(lineStart, '\n');
    if (!lineEnd) lineEnd = lineStart + strlen(lineStart);

    return std::to_string(lineNum) + " | " + std::string(lineStart, lineEnd - lineStart);
}
const std::vector<Error>& ErrorCollector::getErrors() const {
    return errors;
}
