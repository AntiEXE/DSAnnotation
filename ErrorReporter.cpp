#include "ErrorReporter.h"
#include <iostream>
#include <sstream>

ErrorReporter::ErrorReporter(const ErrorCollector& collector) : errorCollector(collector) {}

std::string ErrorReporter::generateSummary() const {
    std::ostringstream summary;
    const auto& errors = errorCollector.getErrors();
    summary << "Following errors occurred during parsing annotations:\n\n";

    for (const auto& error : errors) {
        summary << "Error: " << error.message << "\n"
                << "Location: " << error.location.printToString(errorCollector.getSourceManager()) << "\n"
                << errorCollector.getSourceLine(error.location) << "\n"
                << "Severity: " << static_cast<int>(error.severity) << "\n\n";
    }
    summary << "Total errors: " << errors.size() << "\n\n";
    return summary.str();
}

void ErrorReporter::printReport() const {
    std::cout << generateSummary();
}
