#pragma once

#include <string>

#include "DSAnnotation/Core/ErrorCollector.h"

namespace dsannotation::support {

class ErrorReporter {
public:
    explicit ErrorReporter(const core::ErrorCollector& collector);

    std::string summary() const;
    void print() const;

private:
    const core::ErrorCollector& collector_;
};

} // namespace dsannotation::support
