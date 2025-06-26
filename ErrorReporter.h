#pragma once
#include "ErrorCollector.h"
#include <string>

class ErrorReporter {
public:
    ErrorReporter(const ErrorCollector& collector);
    std::string generateSummary() const;
    void printReport() const;

private:
    const ErrorCollector& errorCollector;
};
