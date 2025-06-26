// ErrorCollector.h
#pragma once
#include <vector>
#include <string>
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"

enum class ErrorCategory {
    COMPONENT,
    REFERENCE,
    PROPERTY,
    GENERAL
};

enum class ErrorSeverity {
    INFO,
    WARNING,
    ERROR
};

struct Error {
    std::string message;
    clang::SourceLocation location;
    ErrorSeverity severity;
    ErrorCategory category;
};

class ErrorCollector {
public:
    ErrorCollector(const clang::SourceManager* SM) : SM(SM) {}
    std::string getSourceLine(const clang::SourceLocation& loc) const;
    void addError(const std::string& message, clang::SourceLocation location, 
                  ErrorSeverity severity, ErrorCategory category);
    const std::vector<Error>& getErrors() const;
    const clang::SourceManager& getSourceManager() const { return *SM; }

private:
    std::vector<Error> errors;
    const clang::SourceManager* SM;

};

