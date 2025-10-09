#include "DSAnnotation/Core/ErrorCollector.h"

#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/raw_ostream.h"

namespace dsannotation::core {

ErrorCollector::ErrorCollector(const clang::SourceManager& sourceManager)
    : sourceManager_(sourceManager) {}

void ErrorCollector::addError(std::string message,
                              clang::SourceLocation location,
                              ErrorSeverity severity,
                              ErrorCategory category) {
    errors_.push_back(Error{std::move(message), formatLocation(location), severity, category});
}

void ErrorCollector::addError(std::string message,
                              const support::SourceLocationInfo& location,
                              ErrorSeverity severity,
                              ErrorCategory category) {
    std::string locationStr;
    if (!location.filename.empty()) {
        locationStr = location.filename + ":" + std::to_string(location.line) + ":" + std::to_string(location.column);
    }
    errors_.push_back(Error{std::move(message), std::move(locationStr), severity, category});
}

void ErrorCollector::addError(std::string message,
                              ErrorSeverity severity,
                              ErrorCategory category) {
    errors_.push_back(Error{std::move(message), std::string{}, severity, category});
}

std::string ErrorCollector::formatLocation(clang::SourceLocation location) const {
    if (!location.isValid()) {
        return "<invalid>";
    }

    auto presumed = sourceManager_.getPresumedLoc(location);
    if (!presumed.isValid()) {
        return "<unknown>";
    }

    std::string buffer;
    llvm::raw_string_ostream stream(buffer);
    stream << presumed.getFilename() << ':' << presumed.getLine() << ':' << presumed.getColumn();
    return stream.str();
}

} // namespace dsannotation::core
