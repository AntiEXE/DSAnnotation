#pragma once

#include <string>
#include <vector>

namespace clang {
class SourceLocation;
class SourceManager;
}

#include "DSAnnotation/Core/Error.h"
#include "DSAnnotation/Support/SourceLocationInfo.h"

namespace dsannotation::core {

class ErrorCollector {
public:
    explicit ErrorCollector(const clang::SourceManager& sourceManager);

    void addError(std::string message,
                  clang::SourceLocation location,
                  ErrorSeverity severity,
                  ErrorCategory category);

    void addError(std::string message,
                  const support::SourceLocationInfo& location,
                  ErrorSeverity severity,
                  ErrorCategory category);

    void addError(std::string message,
                  ErrorSeverity severity,
                  ErrorCategory category);

    const std::vector<Error>& errors() const noexcept { return errors_; }

private:
    std::string formatLocation(clang::SourceLocation location) const;

    const clang::SourceManager& sourceManager_;
    std::vector<Error> errors_;
};

} // namespace dsannotation::core
