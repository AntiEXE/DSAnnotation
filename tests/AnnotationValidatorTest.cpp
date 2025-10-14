#include <gtest/gtest.h>
#include "dsannotation/parsing/AnnotationValidator.h"

namespace dsannotation::parsing::tests {

class AnnotationValidatorTest : public ::testing::Test {
protected:
    AnnotationValidator validator;
    support::SourceLocationInfo testLocation{"test.cpp", 1, 1};
};

TEST_F(AnnotationValidatorTest, ValidatesBalancedBraces) {
    auto result = validator.validateComment("@component{name: \"test\"}", testLocation);
    EXPECT_FALSE(result.hasCriticalErrors());
    EXPECT_TRUE(result.errors.empty());
}

TEST_F(AnnotationValidatorTest, DetectsUnbalancedBraces) {
    auto result = validator.validateComment("@component{name: \"test\"", testLocation);
    EXPECT_TRUE(result.hasCriticalErrors());
    EXPECT_FALSE(result.errors.empty());
    EXPECT_TRUE(result.errors[0].message.find("Unclosed brace") != std::string::npos);
}

TEST_F(AnnotationValidatorTest, DetectsMismatchedBraces) {
    auto result = validator.validateComment("@component{name: \"test\"]", testLocation);
    EXPECT_TRUE(result.hasCriticalErrors());
    EXPECT_FALSE(result.errors.empty());
    EXPECT_TRUE(result.errors[0].message.find("Mismatched braces") != std::string::npos);
}

TEST_F(AnnotationValidatorTest, DetectsUnclosedQuotes) {
    auto result = validator.validateComment("@component{name: \"test}", testLocation);
    EXPECT_TRUE(result.hasCriticalErrors());
    EXPECT_FALSE(result.errors.empty());
    EXPECT_TRUE(result.errors[0].message.find("Unclosed quote") != std::string::npos);
}

TEST_F(AnnotationValidatorTest, ExtractsComponentAnnotation) {
    auto result = validator.validateComment("@component{name: \"TestComponent\"}", testLocation);
    EXPECT_FALSE(result.hasCriticalErrors());
    EXPECT_EQ(result.annotations.size(), 1);
    EXPECT_EQ(result.annotations[0].type, AnnotationType::Component);
    EXPECT_EQ(result.annotations[0].content, "name: \"TestComponent\"");
}

TEST_F(AnnotationValidatorTest, ExtractsMultipleAnnotations) {
    std::string comment = R"(@component{name: "Test"}
@properties{prop1: "value1"}
@reference{interface: "IService"})";
    
    auto result = validator.validateComment(comment, testLocation);
    EXPECT_FALSE(result.hasCriticalErrors());
    EXPECT_EQ(result.annotations.size(), 3);
    
    EXPECT_EQ(result.annotations[0].type, AnnotationType::Component);
    EXPECT_EQ(result.annotations[1].type, AnnotationType::Properties);
    EXPECT_EQ(result.annotations[2].type, AnnotationType::Reference);
}

TEST_F(AnnotationValidatorTest, ValidatesEmptyAnnotations) {
    auto result = validator.validateComment("@component", testLocation);
    EXPECT_FALSE(result.hasCriticalErrors());
    EXPECT_EQ(result.annotations.size(), 1);
    EXPECT_EQ(result.annotations[0].type, AnnotationType::Component);
    EXPECT_TRUE(result.annotations[0].content.empty());
}

TEST_F(AnnotationValidatorTest, RejectsEmptyPropertiesContent) {
    auto result = validator.validateComment("@properties{}", testLocation);
    EXPECT_TRUE(result.hasCriticalErrors());
    EXPECT_FALSE(result.errors.empty());
    EXPECT_TRUE(result.errors[0].message.find("Properties annotation requires content") != std::string::npos);
}

TEST_F(AnnotationValidatorTest, RejectsEmptyReferenceContent) {
    auto result = validator.validateComment("@reference{}", testLocation);
    EXPECT_TRUE(result.hasCriticalErrors());
    EXPECT_FALSE(result.errors.empty());
    EXPECT_TRUE(result.errors[0].message.find("Reference annotation requires content") != std::string::npos);
}

TEST_F(AnnotationValidatorTest, DetectsMultipleComponents) {
    std::string comment = R"(@component{name: "Test1"}
@component{name: "Test2"})";
    
    auto result = validator.validateComment(comment, testLocation);
    EXPECT_TRUE(result.hasCriticalErrors());
    EXPECT_FALSE(result.errors.empty());
    EXPECT_TRUE(result.errors[0].message.find("Multiple @component annotations") != std::string::npos);
}

TEST_F(AnnotationValidatorTest, ValidatesFilePathsForSecurity) {
    auto result = validator.validateComment("@property{\"../../../etc/passwd\"}", testLocation);
    EXPECT_FALSE(result.hasCriticalErrors()); // Should not be critical, but should warn
    EXPECT_FALSE(result.warnings.empty());
    EXPECT_TRUE(result.warnings[0].message.find("path traversal") != std::string::npos);
}

TEST_F(AnnotationValidatorTest, DetectsAnnotationsWithoutComponent) {
    std::string comment = R"(@properties{prop1: "value1"}
@reference{interface: "IService"})";
    
    auto result = validator.validateComment(comment, testLocation);
    EXPECT_TRUE(result.hasCriticalErrors());
    EXPECT_FALSE(result.errors.empty());
    
    // Should find the specific error about missing @component
    bool foundMissingComponentError = false;
    for (const auto& error : result.errors) {
        if (error.message.find("@component") != std::string::npos && 
            error.message.find("required") != std::string::npos) {
            foundMissingComponentError = true;
            break;
        }
    }
    EXPECT_TRUE(foundMissingComponentError);
}

TEST_F(AnnotationValidatorTest, AllowsAnnotationsWithComponent) {
    std::string comment = R"(@component{name: "TestComponent"}
@properties{prop1: "value1"}
@reference{interface: "IService"})";
    
    auto result = validator.validateComment(comment, testLocation);
    EXPECT_FALSE(result.hasCriticalErrors());
    EXPECT_EQ(result.annotations.size(), 3);
}

TEST_F(AnnotationValidatorTest, AllowsComponentAlone) {
    std::string comment = "@component{name: \"TestComponent\"}";
    
    auto result = validator.validateComment(comment, testLocation);
    EXPECT_FALSE(result.hasCriticalErrors());
    EXPECT_EQ(result.annotations.size(), 1);
}

TEST_F(AnnotationValidatorTest, RejectsNullCharacters) {
    std::string commentWithNull = "@component{name: \"test\0bad\"}";
    auto result = validator.validateComment(commentWithNull, testLocation);
    EXPECT_TRUE(result.hasCriticalErrors());
    EXPECT_FALSE(result.errors.empty());
    EXPECT_TRUE(result.errors[0].message.find("Null character") != std::string::npos);
}

TEST_F(AnnotationValidatorTest, RejectsOversizedComments) {
    std::string largeComment(70000, 'x'); // Larger than MAX_COMMENT_LENGTH
    auto result = validator.validateComment(largeComment, testLocation);
    EXPECT_TRUE(result.hasCriticalErrors());
    EXPECT_FALSE(result.errors.empty());
    EXPECT_TRUE(result.errors[0].message.find("exceeds maximum length") != std::string::npos);
}

} // namespace dsannotation::parsing::tests