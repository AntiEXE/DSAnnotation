#pragma once

#include "DSAnnotation/Parsing/AnnotationTypes.h"
#include "DSAnnotation/Support/SourceLocationInfo.h"
#include "DSAnnotation/Core/Error.h"
#include <string>
#include <vector>
#include <regex>

namespace dsannotation::parsing {

class AnnotationValidator {
public:
    AnnotationValidator() = default;
    
    // Main validation entry point
    AnnotationValidationResult validateComment(const std::string& commentText,
                                             const support::SourceLocationInfo& location) const;
    
    // Individual validation methods
    ValidationResult validateSyntax(const std::string& text) const;
    ValidationResult validateAnnotation(const ParsedAnnotation& annotation) const;
    std::vector<ParsedAnnotation> extractAnnotations(const std::string& commentText,
                                                    const support::SourceLocationInfo& baseLocation) const;

private:
    // Basic syntax checks
    bool checkBalancedBraces(const std::string& text, std::vector<ValidationError>& errors) const;
    bool checkBalancedQuotes(const std::string& text, std::vector<ValidationError>& errors) const;
    bool checkValidCharacters(const std::string& text, std::vector<ValidationError>& errors) const;
    
    // Annotation extraction helpers
    std::vector<size_t> findAnnotationPositions(const std::string& text, const std::string& annotationType) const;
    ParsedAnnotation parseAnnotationAt(const std::string& text, size_t position, 
                                      AnnotationType type, const support::SourceLocationInfo& baseLocation) const;
    
    // Utility functions
    support::SourceLocationInfo calculateLocation(const support::SourceLocationInfo& base,
                                                 const std::string& text, size_t position) const;
    size_t findMatchingBrace(const std::string& text, size_t openPos) const;
    std::string extractContent(const std::string& text, size_t start, size_t end) const;
    
    // Constants
    static constexpr size_t MAX_COMMENT_LENGTH = 64 * 1024;
    static constexpr size_t MAX_ANNOTATION_DEPTH = 10;
};

} // namespace dsannotation::parsing