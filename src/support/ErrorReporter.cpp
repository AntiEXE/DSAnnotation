#include "dsannotation/support/ErrorReporter.h"

#include <iostream>
#include <sstream>

namespace dsannotation::support {

ErrorReporter::ErrorReporter(const core::ErrorCollector& collector)
    : collector_(collector) {}

std::string ErrorReporter::summary() const {
    std::ostringstream builder;
    builder << "Following errors occurred during parsing annotations:\n\n";
    const auto& errors = collector_.errors();
    for (const auto& error : errors) {
        builder << "Error: " << error.message << '\n';
        if (!error.location.empty()) {
            builder << "Location: " << error.location << '\n';
        }
        builder << "Severity: " << static_cast<int>(error.severity) << '\n';
        builder << "Category: " << static_cast<int>(error.category) << "\n\n";
    }
    builder << "Total errors: " << errors.size() << "\n";
    return builder.str();
}

void ErrorReporter::print() const {
    std::cout << summary();
}

} // namespace dsannotation::support
