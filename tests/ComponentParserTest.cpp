#include <gtest/gtest.h>
#include "ComponentParser.h"
#include "ErrorCollector.h"
#include "Component.h"

class ComponentParserTest : public ::testing::Test {
protected:
    std::unique_ptr<ErrorCollector> errorCollector;
    std::unique_ptr<ComponentParser> parser;

    void SetUp() override {
        errorCollector = std::make_unique<ErrorCollector>(nullptr);
        parser = std::make_unique<ComponentParser>(*errorCollector);
    }
};

// Note: Testing ComponentParser with real Clang AST objects is complex
// These tests focus on the parts we can test without full AST infrastructure

TEST_F(ComponentParserTest, ComponentParserConstructorWorks) {
    EXPECT_NE(parser.get(), nullptr);
    EXPECT_NE(errorCollector.get(), nullptr);
}

// You might want to add tests for helper functions if they exist
// or create mock tests for the parse functionality
TEST_F(ComponentParserTest, ErrorCollectorIsInitialized) {
    EXPECT_EQ(errorCollector->getErrors().size(), 0);
}

// For more comprehensive testing, you would need to set up a full Clang
// testing environment, which is quite complex. Consider integration tests instead.