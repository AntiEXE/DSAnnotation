#pragma once

#include <string>

namespace dsannotation::core {

enum class ErrorCategory {
    Component,
    Reference,
    Property,
    General,
    IO
};

enum class ErrorSeverity {
    Info,
    Warning,
    Error
};

struct Error {
    std::string message;
    std::string location;
    ErrorSeverity severity;
    ErrorCategory category;
};

} // namespace dsannotation::core
