#include "DSAnnotation/Parsing/AnnotationValidator.h"
#include <algorithm>
#include <stack>
#include <cctype>

namespace dsannotation::parsing {

AnnotationValidationResult AnnotationValidator::validateComment(const std::string& commentText,
                                                              const support::SourceLocationInfo& location) const {
    AnnotationValidationResult result;
    
    // Step 1: Basic syntax validation
    auto syntaxResult = validateSyntax(commentText);
    result.errors.insert(result.errors.end(), syntaxResult.errors.begin(), syntaxResult.errors.end());
    result.warnings.insert(result.warnings.end(), syntaxResult.warnings.begin(), syntaxResult.warnings.end());
    
    if (!syntaxResult.isValid) {
        return result; // Don't continue if basic syntax is invalid
    }
    
    // Step 2: Extract annotations
    result.annotations = extractAnnotations(commentText, location);
    
    // Step 3: Validate each annotation
    for (const auto& annotation : result.annotations) {
        auto annotationResult = validateAnnotation(annotation);
        result.errors.insert(result.errors.end(), annotationResult.errors.begin(), annotationResult.errors.end());
        result.warnings.insert(result.warnings.end(), annotationResult.warnings.begin(), annotationResult.warnings.end());
    }
    
    // Step 4: All annotations extracted and individually validated
    // No cross-annotation business logic - that's handled by the architecture
    
    return result;
}

ValidationResult AnnotationValidator::validateSyntax(const std::string& text) const {
    ValidationResult result;
    result.isValid = true;
    
    // Check length
    if (text.length() > MAX_COMMENT_LENGTH) {
        ValidationError error;
        error.message = "Comment exceeds maximum length of " + std::to_string(MAX_COMMENT_LENGTH) + " characters";
        error.severity = core::ErrorSeverity::Error;
        result.errors.push_back(error);
        result.isValid = false;
    }
    
    // Check for valid characters
    if (!checkValidCharacters(text, result.errors)) {
        result.isValid = false;
    }
    
    // Check balanced braces
    if (!checkBalancedBraces(text, result.errors)) {
        result.isValid = false;
    }
    
    // Check balanced quotes
    if (!checkBalancedQuotes(text, result.errors)) {
        result.isValid = false;
    }
    
    return result;
}

ValidationResult AnnotationValidator::validateAnnotation(const ParsedAnnotation& annotation) const {
    ValidationResult result;
    result.isValid = true;
    
    // Check for malformed annotations (invalid word boundaries)
    if (!annotation.isValid) {
        ValidationError error;
        std::string typeName;
        switch (annotation.type) {
            case AnnotationType::Component: typeName = "@component"; break;
            case AnnotationType::Properties: typeName = "@properties"; break;
            case AnnotationType::Property: typeName = "@property"; break;
            case AnnotationType::Reference: typeName = "@reference"; break;
            default: typeName = "unknown"; break;
        }
        error.message = "Malformed annotation '" + typeName + 
                       "' at line " + std::to_string(annotation.location.line) + 
                       " - not a complete word boundary (e.g., inside 'my" + typeName + "123'). " +
                       "Component processing will be skipped. Ignore, if you didn't intend an annotation.";
        error.severity = core::ErrorSeverity::Error; // Changed from Warning to Error for critical failure
        result.errors.push_back(error);
        result.isValid = false;
        return result; // Don't continue validation for malformed annotations
    }
    
    // Only security-focused validation for file paths in @property annotations
    if (annotation.type == AnnotationType::Property && !annotation.content.empty()) {
        // Check for path traversal in file paths (security concern)
        if (annotation.content.find("..") != std::string::npos) {
            ValidationWarning warning;
            warning.message = "File path contains '..' which may indicate path traversal attempt";
            result.warnings.push_back(warning);
        }
    }
    
    // No other business logic validation - parsers handle content requirements
    return result;
}

std::vector<ParsedAnnotation> AnnotationValidator::extractAnnotations(const std::string& commentText,
                                                                     const support::SourceLocationInfo& baseLocation) const {
    std::vector<ParsedAnnotation> annotations;
    
    // Helper lambda to check for malformed annotations of any type
    auto checkForMalformedAnnotations = [&](const std::string& annotationType, AnnotationType type) {
        size_t pos = 0;
        while ((pos = commentText.find(annotationType, pos)) != std::string::npos) {
            bool validStart = (pos == 0) || !std::isalnum(commentText[pos - 1]);
            bool validEnd = (pos + annotationType.length() >= commentText.length()) || 
                           !std::isalnum(commentText[pos + annotationType.length()]);
            
            if (!validStart || !validEnd) {
                // Found malformed annotation - create invalid annotation to trigger processing failure
                ParsedAnnotation malformed;
                malformed.type = type;
                malformed.location = calculateLocation(baseLocation, commentText, pos);
                malformed.content = "";
                malformed.isValid = false;
                annotations.push_back(malformed);
            }
            pos += annotationType.length();
        }
    };
    
    // Find valid @component annotations
    auto componentPositions = findAnnotationPositions(commentText, "@component");
    for (size_t pos : componentPositions) {
        auto annotation = parseAnnotationAt(commentText, pos, AnnotationType::Component, baseLocation);
        annotations.push_back(annotation);
    }
    // Check for malformed @component annotations
    checkForMalformedAnnotations("@component", AnnotationType::Component);
    
    // Find valid @properties annotations  
    auto propertiesPositions = findAnnotationPositions(commentText, "@properties");
    for (size_t pos : propertiesPositions) {
        auto annotation = parseAnnotationAt(commentText, pos, AnnotationType::Properties, baseLocation);
        annotations.push_back(annotation);
    }
    // Check for malformed @properties annotations
    checkForMalformedAnnotations("@properties", AnnotationType::Properties);
    
    // Find valid @property annotations (for external files)
    auto propertyPositions = findAnnotationPositions(commentText, "@property");
    for (size_t pos : propertyPositions) {
        auto annotation = parseAnnotationAt(commentText, pos, AnnotationType::Property, baseLocation);
        annotations.push_back(annotation);
    }
    // Check for malformed @property annotations
    checkForMalformedAnnotations("@property", AnnotationType::Property);
    
    // Find valid @reference annotations
    auto referencePositions = findAnnotationPositions(commentText, "@reference");
    for (size_t pos : referencePositions) {
        auto annotation = parseAnnotationAt(commentText, pos, AnnotationType::Reference, baseLocation);
        annotations.push_back(annotation);
    }
    // Check for malformed @reference annotations
    checkForMalformedAnnotations("@reference", AnnotationType::Reference);
    
    return annotations;
}

bool AnnotationValidator::checkBalancedBraces(const std::string& text, std::vector<ValidationError>& errors) const {
    std::stack<std::pair<char, size_t>> braceStack;
    bool isValid = true;
    
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        
        if (c == '{' || c == '[' || c == '(') {
            braceStack.push({c, i});
        } else if (c == '}' || c == ']' || c == ')') {
            if (braceStack.empty()) {
                ValidationError error;
                error.message = "Unmatched closing brace '" + std::string(1, c) + "' at position " + std::to_string(i);
                error.severity = core::ErrorSeverity::Error;
                error.position = i;
                errors.push_back(error);
                isValid = false;
                continue;
            }
            
            char openBrace = braceStack.top().first;
            braceStack.pop();
            
            // Check if braces match
            bool matches = (openBrace == '{' && c == '}') ||
                          (openBrace == '[' && c == ']') ||
                          (openBrace == '(' && c == ')');
            
            if (!matches) {
                ValidationError error;
                error.message = "Mismatched braces: '" + std::string(1, openBrace) + "' and '" + std::string(1, c) + "'";
                error.severity = core::ErrorSeverity::Error;
                error.position = i;
                errors.push_back(error);
                isValid = false;
            }
        }
    }
    
    while (!braceStack.empty()) {
        ValidationError error;
        error.message = "Unclosed brace '" + std::string(1, braceStack.top().first) + "'";
        error.severity = core::ErrorSeverity::Error;
        error.position = braceStack.top().second;
        errors.push_back(error);
        braceStack.pop();
        isValid = false;
    }
    
    return isValid;
}

bool AnnotationValidator::checkBalancedQuotes(const std::string& text, std::vector<ValidationError>& errors) const {
    bool inSingleQuote = false;
    bool inDoubleQuote = false;
    bool isValid = true;
    
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        
        if (c == '"' && !inSingleQuote) {
            // Check if escaped
            bool escaped = false;
            if (i > 0 && text[i-1] == '\\') {
                // Count consecutive backslashes
                size_t backslashCount = 0;
                for (int j = static_cast<int>(i) - 1; j >= 0 && text[j] == '\\'; --j) {
                    backslashCount++;
                }
                escaped = (backslashCount % 2 == 1);
            }
            
            if (!escaped) {
                inDoubleQuote = !inDoubleQuote;
            }
        } else if (c == '\'' && !inDoubleQuote) {
            // Similar logic for single quotes
            bool escaped = false;
            if (i > 0 && text[i-1] == '\\') {
                size_t backslashCount = 0;
                for (int j = static_cast<int>(i) - 1; j >= 0 && text[j] == '\\'; --j) {
                    backslashCount++;
                }
                escaped = (backslashCount % 2 == 1);
            }
            
            if (!escaped) {
                inSingleQuote = !inSingleQuote;
            }
        }
    }
    
    if (inDoubleQuote || inSingleQuote) {
        ValidationError error;
        error.message = "Unclosed quote in comment";
        error.severity = core::ErrorSeverity::Error;
        errors.push_back(error);
        isValid = false;
    }
    
    return isValid;
}

bool AnnotationValidator::checkValidCharacters(const std::string& text, std::vector<ValidationError>& errors) const {
    bool isValid = true;
    
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        
        // Check for null characters
        if (c == '\0') {
            ValidationError error;
            error.message = "Null character found at position " + std::to_string(i);
            error.severity = core::ErrorSeverity::Error;
            error.position = i;
            errors.push_back(error);
            isValid = false;
        }
        
        // Check for control characters (except newline, tab, carriage return)
        if (std::iscntrl(c) && c != '\n' && c != '\t' && c != '\r') {
            ValidationError error;
            error.message = "Invalid control character found at position " + std::to_string(i);
            error.severity = core::ErrorSeverity::Warning;
            error.position = i;
            errors.push_back(error);
        }
    }
    
    return isValid;
}

std::vector<size_t> AnnotationValidator::findAnnotationPositions(const std::string& text, const std::string& annotationType) const {
    std::vector<size_t> positions;
    size_t pos = 0;
    
    while ((pos = text.find(annotationType, pos)) != std::string::npos) {
        // Check if it's a word boundary (not part of another word)
        bool validStart = (pos == 0) || !std::isalnum(text[pos - 1]);
        bool validEnd = (pos + annotationType.length() >= text.length()) || 
                       !std::isalnum(text[pos + annotationType.length()]);
        
        if (validStart && validEnd) {
            positions.push_back(pos);
        } else {
            // This is a malformed annotation (e.g., @component inside my@component123)
            // We need to report this as an error rather than silently ignoring it
            ValidationError error;
            error.message = "Malformed annotation '" + annotationType + "' found at position " + 
                           std::to_string(pos) + " - not a complete word boundary";
            error.severity = core::ErrorSeverity::Warning;
            error.position = pos;
            // Note: We can't add to errors vector here since this is a const method
            // This needs architectural improvement - see extractAnnotations
        }
        pos += annotationType.length();
    }
    
    return positions;
}

ParsedAnnotation AnnotationValidator::parseAnnotationAt(const std::string& text, size_t position,
                                                       AnnotationType type, const support::SourceLocationInfo& baseLocation) const {
    ParsedAnnotation annotation;
    annotation.type = type;
    annotation.location = calculateLocation(baseLocation, text, position);
    
    // Find the annotation name end
    size_t nameEnd = position;
    while (nameEnd < text.length() && (std::isalnum(text[nameEnd]) || text[nameEnd] == '@')) {
        nameEnd++;
    }
    
    // Skip whitespace
    while (nameEnd < text.length() && std::isspace(text[nameEnd])) {
        nameEnd++;
    }
    
    // Check if there's content in braces
    if (nameEnd < text.length() && text[nameEnd] == '{') {
        size_t braceEnd = findMatchingBrace(text, nameEnd);
        if (braceEnd != std::string::npos) {
            annotation.content = extractContent(text, nameEnd + 1, braceEnd);
            annotation.isValid = true;
        } else {
            annotation.content = "";
            annotation.isValid = false;
        }
    } else {
        // No content (empty annotation)
        annotation.content = "";
        annotation.isValid = true;
    }
    
    return annotation;
}

support::SourceLocationInfo AnnotationValidator::calculateLocation(const support::SourceLocationInfo& base,
                                                                  const std::string& text, size_t position) const {
    unsigned int line = base.line;
    unsigned int column = base.column;
    
    for (size_t i = 0; i < position && i < text.length(); ++i) {
        if (text[i] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
    }
    
    return support::SourceLocationInfo(base.filename, line, column);
}

size_t AnnotationValidator::findMatchingBrace(const std::string& text, size_t openPos) const {
    if (openPos >= text.length() || text[openPos] != '{') {
        return std::string::npos;
    }
    
    std::stack<char> braceStack;
    braceStack.push('{');
    
    for (size_t i = openPos + 1; i < text.length(); ++i) {
        char c = text[i];
        
        if (c == '{') {
            braceStack.push('{');
        } else if (c == '}') {
            braceStack.pop();
            if (braceStack.empty()) {
                return i;
            }
        }
    }
    
    return std::string::npos;
}

std::string AnnotationValidator::extractContent(const std::string& text, size_t start, size_t end) const {
    if (start >= end || start >= text.length()) {
        return "";
    }
    
    size_t actualEnd = std::min(end, text.length());
    return text.substr(start, actualEnd - start);
}

} // namespace dsannotation::parsing