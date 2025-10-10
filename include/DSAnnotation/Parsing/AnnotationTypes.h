#pragma once

#include <string>
#include <vector>
#include <optional>
#include "nlohmann/json.hpp"
#include "DSAnnotation/Support/SourceLocationInfo.h"
#include "DSAnnotation/Core/Error.h"

namespace dsannotation::parsing {

enum class AnnotationType {
    Component,
    Properties,
    Property,      // External file reference
    Reference,
    Unknown
};

struct ValidationError {
    std::string message;
    std::string suggestion;
    size_t position{0};           // Character position within text
    core::ErrorSeverity severity{core::ErrorSeverity::Error};
};

struct ValidationWarning {
    std::string message;
    std::string suggestion;
    size_t position{0};
};

struct ValidationResult {
    bool isValid{true};
    std::vector<ValidationError> errors;
    std::vector<ValidationWarning> warnings;
    
    bool hasCriticalErrors() const {
        return !isValid || std::any_of(errors.begin(), errors.end(),
                                      [](const auto& err) { 
                                          return err.severity == core::ErrorSeverity::Error; 
                                      });
    }
};

struct ParsedAnnotation {
    AnnotationType type{AnnotationType::Unknown};
    std::string content;                    // Raw content inside braces
    support::SourceLocationInfo location;  // Specific location within comment
    nlohmann::json parsedContent;          // Pre-parsed content if valid
    bool isValid{false};                   // Whether parsing succeeded
    
    ParsedAnnotation() = default;
    ParsedAnnotation(AnnotationType t, std::string c, support::SourceLocationInfo loc)
        : type(t), content(std::move(c)), location(std::move(loc)) {}
};

struct AnnotationValidationResult {
    std::vector<ParsedAnnotation> annotations;
    std::vector<ValidationError> errors;
    std::vector<ValidationWarning> warnings;
    
    bool hasCriticalErrors() const {
        return std::any_of(errors.begin(), errors.end(),
                          [](const auto& err) { 
                              return err.severity == core::ErrorSeverity::Error; 
                          });
    }
    
    void addError(ValidationError error) {
        errors.push_back(std::move(error));
    }
    
    void addWarning(ValidationWarning warning) {
        warnings.push_back(std::move(warning));
    }
};

} // namespace dsannotation::parsing