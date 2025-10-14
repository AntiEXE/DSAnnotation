#pragma once

#include <string>
#include <vector>
#include "dsannotation/parsing/AnnotationTypes.h"
#include "dsannotation/support/SourceLocationInfo.h"

namespace dsannotation::parsing {

class IAnnotationValidator {
public:
    virtual ~IAnnotationValidator() = default;
    
    /// Validates basic syntax (braces, quotes, null characters)
    virtual ValidationResult validateBasicSyntax(const std::string& text,
                                                 const support::SourceLocationInfo& location) const = 0;
    
    /// Extracts and validates individual annotations from comment text
    virtual AnnotationValidationResult validateAndParseAnnotations(const std::string& commentText,
                                                                   const support::SourceLocationInfo& baseLocation) const = 0;
    
    /// Validates structure of a specific annotation type
    virtual ValidationResult validateAnnotationStructure(ParsedAnnotation& annotation) const = 0;
    
    /// Validates content values within an annotation
    virtual ValidationResult validateAnnotationContent(const ParsedAnnotation& annotation) const = 0;
    
    /// Validates consistency across multiple annotations
    virtual ValidationResult validateAnnotationConsistency(const std::vector<ParsedAnnotation>& annotations) const = 0;
};

} // namespace dsannotation::parsing