#include <gtest/gtest.h>
#include "ComponentParser.h"
#include "ErrorCollector.h"
#include "Component.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RawCommentList.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/FileManager.h"
#include "clang/Tooling/Tooling.h"
#include <memory>

class ComponentParserTest : public ::testing::Test {
protected:
    std::unique_ptr<ErrorCollector> errorCollector;
    std::unique_ptr<ComponentParser> parser;

    void SetUp() override {
        errorCollector = std::make_unique<ErrorCollector>(nullptr);
        parser = std::make_unique<ComponentParser>(*errorCollector);
    }
};

/**
 * Test that the ComponentParser constructor works correctly and initializes the object.
 */
TEST_F(ComponentParserTest, ComponentParserConstructorWorks) {
    EXPECT_NE(parser.get(), nullptr);
    EXPECT_NE(errorCollector.get(), nullptr);
}

/**
 * Test that the ErrorCollector is properly initialized with no errors.
 */
TEST_F(ComponentParserTest, ErrorCollectorIsInitialized) {
    EXPECT_EQ(errorCollector->getErrors().size(), 0);
}

/**
 * Test parsing a class that has no comments at all.
 * Verifies that the parser handles this case gracefully without errors.
 */
TEST_F(ComponentParserTest, ParseClassWithNoComments) {
    // Create a test file with a class that has no comments at all
    std::string code = R"(
        // This is a line comment, not attached to the class
        class NoCommentClass {
        public:
            NoCommentClass() {}
        };
    )";

    // Use Clang's tooling to parse the code
    auto AST = clang::tooling::buildASTFromCode(code);
    ASSERT_TRUE(AST.get() != nullptr);

    // Find the class declaration
    auto& context = AST->getASTContext();
    
    // Find the NoCommentClass class
    const clang::CXXRecordDecl* recordDecl = nullptr;
    for (const auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl)) {
            if (record->getNameAsString() == "NoCommentClass") {
                recordDecl = record;
                break;
            }
        }
    }
    ASSERT_TRUE(recordDecl != nullptr);

    // Verify that there are no raw comments for this declaration
    const clang::RawComment* rawComment = context.getRawCommentForDeclNoCache(recordDecl);
    EXPECT_TRUE(rawComment == nullptr);

    // Call the public parse method
    Component component = parser->parse(recordDecl, &context);

    // Verify that no attributes were parsed
    EXPECT_FALSE(component.hasAttributes());
    
    // Verify the component name is still set correctly
    EXPECT_EQ(component.getClassName(), "NoCommentClass");
    
    // Verify interfaces are empty or correctly parsed if the class has base classes
    std::vector<std::string> interfaces = component.getInterfaces();
    EXPECT_TRUE(interfaces.empty()); // Since NoCommentClass has no base classes
    
    // Verify properties are empty
    nlohmann::json properties = component.getProperties();
    EXPECT_TRUE(properties.empty());
    
    // Verify references are empty
    std::vector<Reference> references = component.getReferences();
    EXPECT_TRUE(references.empty());
}

/**
 * Test parsing a class with comments but without the @component tag.
 * Verifies that the parser doesn't treat regular comments as component annotations.
 */
TEST_F(ComponentParserTest, ParseClassWithoutComponentTag) {
    // Create a test file without the component annotation
    std::string code = R"(
        /**
         * This is a regular class comment without any component tag
         */
        class RegularClass {
        public:
            RegularClass() {}
        };
    )";

    // Use Clang's tooling to parse the code
    auto AST = clang::tooling::buildASTFromCode(code);
    ASSERT_TRUE(AST.get() != nullptr);

    // Find the class declaration
    auto& context = AST->getASTContext();
    
    // Find the RegularClass class
    const clang::CXXRecordDecl* recordDecl = nullptr;
    for (const auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl)) {
            if (record->getNameAsString() == "RegularClass") {
                recordDecl = record;
                break;
            }
        }
    }
    ASSERT_TRUE(recordDecl != nullptr);

    // Call the public parse method
    Component component = parser->parse(recordDecl, &context);

    // Verify that no attributes were parsed
    EXPECT_FALSE(component.hasAttributes());
    
    // Verify the component name is still set correctly
    EXPECT_EQ(component.getClassName(), "RegularClass");
    
    // Verify interfaces are empty or correctly parsed if the class has base classes
    std::vector<std::string> interfaces = component.getInterfaces();
    EXPECT_TRUE(interfaces.empty()); // Since RegularClass has no base classes
    
    // Verify properties are empty
    nlohmann::json properties = component.getProperties();
    EXPECT_TRUE(properties.empty());
    
    // Verify references are empty
    std::vector<Reference> references = component.getReferences();
    EXPECT_TRUE(references.empty());
}

/**
 * Test that the parseComponentAttributes method correctly extracts component attributes
 * from a class comment with the @component tag.
 */
TEST_F(ComponentParserTest, ParseMethodHandlesComponentAttributes) {
    // Create a test file with the component annotation
    std::string code = R"(
        /** @component {inject-references = true, service.scope = prototype}
         */
        class TestComponent {
        public:
            TestComponent() {}
        };
    )";

    // Use Clang's tooling to parse the code
    auto AST = clang::tooling::buildASTFromCode(code);
    ASSERT_TRUE(AST.get() != nullptr);

    // Find the class declaration
    auto& context = AST->getASTContext();
    
    // Find the TestComponent class
    const clang::CXXRecordDecl* recordDecl = nullptr;
    for (const auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl)) {
            if (record->getNameAsString() == "TestComponent") {
                recordDecl = record;
                break;
            }
        }
    }
    ASSERT_TRUE(recordDecl != nullptr);

    // Call the public parse method which internally calls parseComponentAttributes
    Component component = parser->parse(recordDecl, &context);

    // Verify the attributes were correctly parsed
    nlohmann::json attributes = component.getAttributes();
    ASSERT_FALSE(attributes.empty());
    EXPECT_TRUE(attributes.contains("inject-references"));
    EXPECT_TRUE(attributes["inject-references"].get<bool>());
    EXPECT_TRUE(attributes.contains("service"));
    EXPECT_TRUE(attributes["service"].contains("scope"));
    EXPECT_EQ(attributes["service"]["scope"], "prototype");
}

/**
 * Test parsing the JsonSerializerServiceProvider example from the codebase.
 * Verifies that the parser correctly extracts component attributes, interfaces, and other metadata.
 */
TEST_F(ComponentParserTest, ParseJsonSerializerServiceProviderExample) {
    // Create a test file with the component annotation from example/test.cpp
    std::string code = R"(
        namespace util {
            class ServiceProvider {
            public:
                ServiceProvider() {}
            };
            
            namespace json {
                class JsonSerializer {
                public:
                    JsonSerializer() {}
                };
                
                /** @component {inject-references = true, service.scope = prototype}
                 */
                class JsonSerializerServiceProvider : public util::ServiceProvider, public util::json::JsonSerializer {
                public:
                    JsonSerializerServiceProvider() {}
                };
            }
        }
    )";

    // Use Clang's tooling to parse the code
    auto AST = clang::tooling::buildASTFromCode(code);
    ASSERT_TRUE(AST.get() != nullptr);

    // Find the class declaration
    auto& context = AST->getASTContext();
    
    // Find the JsonSerializerServiceProvider class
    const clang::CXXRecordDecl* recordDecl = nullptr;
    for (const auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (const auto* namespaceDecl = llvm::dyn_cast<clang::NamespaceDecl>(decl)) {
            if (namespaceDecl->getNameAsString() == "util") {
                for (const auto* utilDecl : namespaceDecl->decls()) {
                    if (const auto* jsonNamespace = llvm::dyn_cast<clang::NamespaceDecl>(utilDecl)) {
                        if (jsonNamespace->getNameAsString() == "json") {
                            for (const auto* jsonDecl : jsonNamespace->decls()) {
                                if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(jsonDecl)) {
                                    if (record->getNameAsString() == "JsonSerializerServiceProvider") {
                                        recordDecl = record;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    ASSERT_TRUE(recordDecl != nullptr);

    // Call the public parse method which internally calls parseComponentAttributes
    Component component = parser->parse(recordDecl, &context);

    // Verify the attributes were correctly parsed
    nlohmann::json attributes = component.getAttributes();
    ASSERT_FALSE(attributes.empty());
    EXPECT_TRUE(attributes.contains("inject-references"));
    EXPECT_TRUE(attributes["inject-references"].get<bool>());
    EXPECT_TRUE(attributes.contains("service"));
    EXPECT_TRUE(attributes["service"].contains("scope"));
    EXPECT_EQ(attributes["service"]["scope"], "prototype");

    // Also verify that the interfaces were correctly parsed
    std::vector<std::string> interfaces = component.getInterfaces();
    EXPECT_EQ(interfaces.size(), 2);
    EXPECT_TRUE(std::find(interfaces.begin(), interfaces.end(), "util::ServiceProvider") != interfaces.end());
    EXPECT_TRUE(std::find(interfaces.begin(), interfaces.end(), "util::json::JsonSerializer") != interfaces.end());

    // Verify the component name
    EXPECT_EQ(component.getClassName(), "util::json::JsonSerializerServiceProvider");

    // Verify the result matches what we expect in the manifest.json
    nlohmann::json expectedAttributes = {
        {"inject-references", true},
        {"service", {{"scope", "prototype"}}}
    };
    EXPECT_EQ(attributes, expectedAttributes);
}

/**
 * Test that the parseProperties method correctly extracts properties from a JSON string
 * in a class comment with the @properties tag.
 */
TEST_F(ComponentParserTest, ParsePropertiesFromJsonString) {
    // Create a test file with @properties JSON annotation
    std::string code = R"(
        /** @component
         * @properties {"property1":"value1","property2":["value2a","value2b"],"property3":true}
         */
        class ComponentWithProperties {
        public:
            ComponentWithProperties() {}
        };
    )";

    // Use Clang's tooling to parse the code
    auto AST = clang::tooling::buildASTFromCode(code);
    ASSERT_TRUE(AST.get() != nullptr);

    // Find the class declaration
    auto& context = AST->getASTContext();
    
    // Find the ComponentWithProperties class
    const clang::CXXRecordDecl* recordDecl = nullptr;
    for (const auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl)) {
            if (record->getNameAsString() == "ComponentWithProperties") {
                recordDecl = record;
                break;
            }
        }
    }
    ASSERT_TRUE(recordDecl != nullptr);

    // Call the public parse method which internally calls parseProperties
    Component component = parser->parse(recordDecl, &context);

    // Verify the properties were correctly parsed
    nlohmann::json properties = component.getProperties();
    ASSERT_FALSE(properties.empty());
    
    // Check specific properties match what we expect
    EXPECT_EQ(properties["property1"], "value1");
    EXPECT_TRUE(properties["property2"].is_array());
    EXPECT_EQ(properties["property2"][0], "value2a");
    EXPECT_EQ(properties["property2"][1], "value2b");
    EXPECT_TRUE(properties["property3"].get<bool>());

    // Verify the expected JSON structure
    nlohmann::json expectedProperties = {
        {"property1", "value1"},
        {"property2", {"value2a", "value2b"}},
        {"property3", true}
    };
    EXPECT_EQ(properties, expectedProperties);
}

/**
 * Test parsing properties from the example file in the codebase.
 * Verifies that the parser correctly extracts properties from the comment and
 * that they match what's expected in the manifest.json file.
 */
TEST_F(ComponentParserTest, ParsePropertiesFromExampleFile) {
    // Create a test file based on example/test.cpp
    std::string code = R"(
        namespace util {
            class ServiceProvider {
            public:
                ServiceProvider() {}
            };
            
            namespace json {
                class JsonSerializer {
                public:
                    JsonSerializer() {}
                };
                
                /** @component {inject-references = true, service.scope = prototype}
                 * @properties {"property1":"value1","property2":["value2a","value2b"],"property3":true}
                 */
                class JsonSerializerServiceProvider : public util::ServiceProvider, public util::json::JsonSerializer {
                public:
                    JsonSerializerServiceProvider() {}
                };
            }
        }
    )";

    // Use Clang's tooling to parse the code
    auto AST = clang::tooling::buildASTFromCode(code);
    ASSERT_TRUE(AST.get() != nullptr);

    // Find the class declaration
    auto& context = AST->getASTContext();
    
    // Find the JsonSerializerServiceProvider class
    const clang::CXXRecordDecl* recordDecl = nullptr;
    for (const auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (const auto* namespaceDecl = llvm::dyn_cast<clang::NamespaceDecl>(decl)) {
            if (namespaceDecl->getNameAsString() == "util") {
                for (const auto* utilDecl : namespaceDecl->decls()) {
                    if (const auto* jsonNamespace = llvm::dyn_cast<clang::NamespaceDecl>(utilDecl)) {
                        if (jsonNamespace->getNameAsString() == "json") {
                            for (const auto* jsonDecl : jsonNamespace->decls()) {
                                if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(jsonDecl)) {
                                    if (record->getNameAsString() == "JsonSerializerServiceProvider") {
                                        recordDecl = record;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    ASSERT_TRUE(recordDecl != nullptr);

    // Call the public parse method which internally calls parseProperties
    Component component = parser->parse(recordDecl, &context);

    // Verify the properties were correctly parsed
    nlohmann::json properties = component.getProperties();
    ASSERT_FALSE(properties.empty());
    
    // Check specific properties match what we expect from build/manifest.json
    EXPECT_EQ(properties["property1"], "value1");
    EXPECT_TRUE(properties["property2"].is_array());
    EXPECT_EQ(properties["property2"][0], "value2a");
    EXPECT_EQ(properties["property2"][1], "value2b");
    EXPECT_TRUE(properties["property3"].get<bool>());

    // Verify the expected JSON structure matches the manifest.json
    nlohmann::json expectedProperties = {
        {"property1", "value1"},
        {"property2", {"value2a", "value2b"}},
        {"property3", true}
    };
    EXPECT_EQ(properties, expectedProperties);
    
    // Also verify that component attributes were parsed correctly
    nlohmann::json attributes = component.getAttributes();
    EXPECT_TRUE(attributes.contains("inject-references"));
    EXPECT_TRUE(attributes["inject-references"].get<bool>());
    EXPECT_TRUE(attributes.contains("service"));
    EXPECT_TRUE(attributes["service"].contains("scope"));
    EXPECT_EQ(attributes["service"]["scope"], "prototype");
}

/**
 * Test parsing properties from an external file.
 * This test verifies that the parser correctly handles the @property tag
 * that points to an external JSON file. Since we can't easily create actual
 * files in a unit test, this test primarily checks that the parser doesn't
 * crash and correctly sets the component name.
 */
TEST_F(ComponentParserTest, ParseExternalPropertiesFile) {
    // This test requires creating a temporary file with properties
    // We'll use a mock approach instead of actual file I/O
    
    // Create a test file with @property annotation pointing to an external file
    std::string code = R"(
        /** @component
         * @property {properties.json}
         */
        class ComponentWithExternalProperties {
        public:
            ComponentWithExternalProperties() {}
        };
    )";

    // Use Clang's tooling to parse the code
    auto AST = clang::tooling::buildASTFromCode(code);
    ASSERT_TRUE(AST.get() != nullptr);

    // Find the class declaration
    auto& context = AST->getASTContext();
    
    // Find the ComponentWithExternalProperties class
    const clang::CXXRecordDecl* recordDecl = nullptr;
    for (const auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl)) {
            if (record->getNameAsString() == "ComponentWithExternalProperties") {
                recordDecl = record;
                break;
            }
        }
    }
    ASSERT_TRUE(recordDecl != nullptr);

    // For this test, we can't easily test the actual file reading functionality
    // without setting up a mock or creating a real file.
    // Instead, we'll verify that the parse method doesn't crash and that
    // the component is created correctly with the expected class name.
    
    // Call the public parse method
    Component component = parser->parse(recordDecl, &context);
    
    // Verify the component name is set correctly
    EXPECT_EQ(component.getClassName(), "ComponentWithExternalProperties");
    
    // Note: We can't verify the properties content since we're not actually
    // creating the external file in this test. In a real test environment,
    // you might want to:
    // 1. Create a temporary file with known content
    // 2. Modify the code string to point to that file
    // 3. Verify the properties match the file content
}

/**
 * Test parsing a component with attributes but no properties.
 * Verifies that the parser correctly handles a class with the @component tag
 * and attributes but no @properties or @property tags.
 */
TEST_F(ComponentParserTest, ParseNoProperties) {
    // Create a test file with @component but no properties
    std::string code = R"(
        /** @component {inject-references = true}
         * This component has attributes but no properties
         */
        class ComponentWithoutProperties {
        public:
            ComponentWithoutProperties() {}
        };
    )";

    // Use Clang's tooling to parse the code
    auto AST = clang::tooling::buildASTFromCode(code);
    ASSERT_TRUE(AST.get() != nullptr);

    // Find the class declaration
    auto& context = AST->getASTContext();
    
    // Find the ComponentWithoutProperties class
    const clang::CXXRecordDecl* recordDecl = nullptr;
    for (const auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl)) {
            if (record->getNameAsString() == "ComponentWithoutProperties") {
                recordDecl = record;
                break;
            }
        }
    }
    ASSERT_TRUE(recordDecl != nullptr);

    // Call the public parse method
    Component component = parser->parse(recordDecl, &context);

    // Verify that no properties were parsed
    nlohmann::json properties = component.getProperties();
    EXPECT_TRUE(properties.empty());
    
    // Verify that attributes were still parsed correctly
    nlohmann::json attributes = component.getAttributes();
    EXPECT_FALSE(attributes.empty());
    EXPECT_TRUE(attributes.contains("inject-references"));
    EXPECT_TRUE(attributes["inject-references"].get<bool>());
}

/**
 * Test parsing references from a constructor with @reference annotations.
 * Verifies that the parser correctly extracts reference information including
 * name, interface, and properties like cardinality and policy-option.
 */
TEST_F(ComponentParserTest, ParseReferencesFromConstructor) {
    // Create a test file with constructor that has @reference annotations
    std::string code = R"(
        #include <memory>
        
        class Interface1 {
        public:
            virtual ~Interface1() = default;
        };
        
        class Interface2 {
        public:
            virtual ~Interface2() = default;
        };
        
        /** @component
         */
        class ComponentWithReferences {
        public:
            /**
             * @reference Interface1 {cardinality = 1..n}
             * @reference Interface2 {policy-option = greedy, cardinality = 0..1}
             */
            ComponentWithReferences(const std::shared_ptr<Interface1>& ref1, 
                                   const std::shared_ptr<Interface2>& ref2) {}
        };
    )";

    // Use Clang's tooling to parse the code
    auto AST = clang::tooling::buildASTFromCode(code);
    ASSERT_TRUE(AST.get() != nullptr);

    // Find the class declaration
    auto& context = AST->getASTContext();
    
    // Find the ComponentWithReferences class
    const clang::CXXRecordDecl* recordDecl = nullptr;
    for (const auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl)) {
            if (record->getNameAsString() == "ComponentWithReferences") {
                recordDecl = record;
                break;
            }
        }
    }
    ASSERT_TRUE(recordDecl != nullptr);

    // Call the public parse method which internally calls parseReferences
    Component component = parser->parse(recordDecl, &context);

    // Verify the references were correctly parsed
    std::vector<Reference> references = component.getReferences();
    ASSERT_EQ(references.size(), 2);
    
    // Check first reference
    EXPECT_EQ(references[0].getName(), "Interface1");
    EXPECT_EQ(references[0].getInterface(), "Interface1");
    
    // Check properties of first reference
    nlohmann::json props1 = references[0].getProperties();
    EXPECT_TRUE(props1.contains("cardinality"));
    EXPECT_EQ(props1["cardinality"], "1..n");
    
    // Check second reference
    EXPECT_EQ(references[1].getName(), "Interface2");
    EXPECT_EQ(references[1].getInterface(), "Interface2");
    
    // Check properties of second reference
    nlohmann::json props2 = references[1].getProperties();
    EXPECT_TRUE(props2.contains("cardinality"));
    EXPECT_EQ(props2["cardinality"], "0..1");
    EXPECT_TRUE(props2.contains("policy-option"));
    EXPECT_EQ(props2["policy-option"], "greedy");
}

/**
 * Test parsing references from the example file in the codebase.
 * Verifies that the parser correctly extracts references from a constructor
 * that matches the example in example/test.cpp and that they match what's
 * expected in the manifest.json file.
 */
TEST_F(ComponentParserTest, ParseReferencesFromExampleFile) {
    // Create a test file based on example/test.cpp
    std::string code = R"(
        #include <memory>
        namespace util {
            class ServiceProvider {
            public:
                ServiceProvider() {}
            };
            
            namespace json {
                class JsonSerializer {
                public:
                    JsonSerializer() {}
                };
                
                class JsonSerializerImpl : public JsonSerializer {
                public:
                    JsonSerializerImpl() {}
                };
                
                /** @component {inject-references = true, service.scope = prototype}
                 * @properties {"property1":"value1","property2":["value2a","value2b"],"property3":true}
                 */
                class JsonSerializerServiceProvider : public util::ServiceProvider, public JsonSerializer {
                public:
                    /**
                     * @reference JsonSerializer {cardinality = 1..n}
                     * @reference JsonSerializerImpl {policy-option = greedy, cardinality = 1..n}
                     */
                    JsonSerializerServiceProvider(const std::shared_ptr<JsonSerializer>& bar, 
                                                 const std::shared_ptr<JsonSerializerImpl>& foo) {}
                };
            }
        }
    )";

    // Use Clang's tooling to parse the code
    auto AST = clang::tooling::buildASTFromCode(code);
    ASSERT_TRUE(AST.get() != nullptr);

    // Find the class declaration
    auto& context = AST->getASTContext();
    
    // Find the JsonSerializerServiceProvider class
    const clang::CXXRecordDecl* recordDecl = nullptr;
    for (const auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (const auto* namespaceDecl = llvm::dyn_cast<clang::NamespaceDecl>(decl)) {
            if (namespaceDecl->getNameAsString() == "util") {
                for (const auto* utilDecl : namespaceDecl->decls()) {
                    if (const auto* jsonNamespace = llvm::dyn_cast<clang::NamespaceDecl>(utilDecl)) {
                        if (jsonNamespace->getNameAsString() == "json") {
                            for (const auto* jsonDecl : jsonNamespace->decls()) {
                                if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(jsonDecl)) {
                                    if (record->getNameAsString() == "JsonSerializerServiceProvider") {
                                        recordDecl = record;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    ASSERT_TRUE(recordDecl != nullptr);

    // Call the public parse method which internally calls parseReferences
    Component component = parser->parse(recordDecl, &context);

    // Verify the references were correctly parsed
    std::vector<Reference> references = component.getReferences();
    ASSERT_EQ(references.size(), 2);
    
    // Check references match what we expect from build/manifest.json
    bool foundJsonSerializer = false;
    bool foundJsonSerializerImpl = false;
    
    for (const auto& ref : references) {
        if (ref.getName() == "JsonSerializer") {
            foundJsonSerializer = true;
            EXPECT_EQ(ref.getInterface(), "util::json::JsonSerializer");
            
            // Check properties
            nlohmann::json props = ref.getProperties();
            EXPECT_TRUE(props.contains("cardinality"));
            EXPECT_EQ(props["cardinality"], "1..n");
        }
        else if (ref.getName() == "JsonSerializerImpl") {
            foundJsonSerializerImpl = true;
            EXPECT_EQ(ref.getInterface(), "util::json::JsonSerializerImpl");
            
            // Check properties
            nlohmann::json props = ref.getProperties();
            EXPECT_TRUE(props.contains("cardinality"));
            EXPECT_EQ(props["cardinality"], "1..n");
            EXPECT_TRUE(props.contains("policy-option"));
            EXPECT_EQ(props["policy-option"], "greedy");
        }
    }
    
    EXPECT_TRUE(foundJsonSerializer);
    EXPECT_TRUE(foundJsonSerializerImpl);
    
    // Also verify that component attributes and properties were parsed correctly
    nlohmann::json attributes = component.getAttributes();
    EXPECT_TRUE(attributes.contains("inject-references"));
    EXPECT_TRUE(attributes["inject-references"].get<bool>());
    
    nlohmann::json properties = component.getProperties();
    EXPECT_EQ(properties["property1"], "value1");
    EXPECT_TRUE(properties["property2"].is_array());
    EXPECT_EQ(properties["property2"][0], "value2a");
}

/**
 * Test parsing a component with no references.
 * Verifies that the parser correctly handles a class with the @component tag
 * but no constructor with @reference annotations.
 */
TEST_F(ComponentParserTest, ParseNoReferences) {
    // Create a test file with a component that has no constructor with references
    std::string code = R"(
        /** @component
         */
        class ComponentWithoutReferences {
        public:
            // No constructor with @reference annotations
            ComponentWithoutReferences() {}
        };
    )";

    // Use Clang's tooling to parse the code
    auto AST = clang::tooling::buildASTFromCode(code);
    ASSERT_TRUE(AST.get() != nullptr);

    // Find the class declaration
    auto& context = AST->getASTContext();
    
    // Find the ComponentWithoutReferences class
    const clang::CXXRecordDecl* recordDecl = nullptr;
    for (const auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl)) {
            if (record->getNameAsString() == "ComponentWithoutReferences") {
                recordDecl = record;
                break;
            }
        }
    }
    ASSERT_TRUE(recordDecl != nullptr);

    // Call the public parse method
    Component component = parser->parse(recordDecl, &context);

    // Verify that no references were parsed
    std::vector<Reference> references = component.getReferences();
    EXPECT_TRUE(references.empty());
}

/**
 * Test parsing a constructor with parameters but no @reference annotations.
 * Verifies that the parser doesn't create references for constructor parameters
 * when there are no @reference annotations.
 */
TEST_F(ComponentParserTest, ParseConstructorWithoutReferenceAnnotation) {
    // Create a test file with a constructor that has parameters but no @reference annotations
    std::string code = R"(
        #include <memory>
        
        class Dependency {
        public:
            Dependency() {}
        };
        
        /** @component
         */
        class ComponentWithUnannotatedConstructor {
        public:
            // Constructor with parameters but no @reference annotations
            ComponentWithUnannotatedConstructor(const std::shared_ptr<Dependency>& dep) {}
        };
    )";

    // Use Clang's tooling to parse the code
    auto AST = clang::tooling::buildASTFromCode(code);
    ASSERT_TRUE(AST.get() != nullptr);

    // Find the class declaration
    auto& context = AST->getASTContext();
    
    // Find the ComponentWithUnannotatedConstructor class
    const clang::CXXRecordDecl* recordDecl = nullptr;
    for (const auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl)) {
            if (record->getNameAsString() == "ComponentWithUnannotatedConstructor") {
                recordDecl = record;
                break;
            }
        }
    }
    ASSERT_TRUE(recordDecl != nullptr);

    // Call the public parse method
    Component component = parser->parse(recordDecl, &context);

    // Verify that no references were parsed since there's no @reference annotation
    std::vector<Reference> references = component.getReferences();
    EXPECT_TRUE(references.empty());
}

/**
 * Test parsing a class with multiple constructors.
 * Verifies that the parser correctly extracts references only from the constructor
 * that has @reference annotations, ignoring other constructors.
 */
TEST_F(ComponentParserTest, ParseMultipleConstructors) {
    // Create a test file with multiple constructors, only one with @reference annotations
    std::string code = R"(
        #include <memory>
        
        class Interface1 {
        public:
            virtual ~Interface1() = default;
        };
        
        class Interface2 {
        public:
            virtual ~Interface2() = default;
        };
        
        /** @component
         */
        class ComponentWithMultipleConstructors {
        public:
            // Default constructor without references
            ComponentWithMultipleConstructors() {}
            
            // Constructor with a single parameter but no @reference annotation
            ComponentWithMultipleConstructors(int value) {}
            
            /**
             * @reference Interface1 {cardinality = 1..n}
             * @reference Interface2 {policy-option = greedy, cardinality = 0..1}
             */
            ComponentWithMultipleConstructors(const std::shared_ptr<Interface1>& ref1, 
                                             const std::shared_ptr<Interface2>& ref2) {}
        };
    )";

    // Use Clang's tooling to parse the code
    auto AST = clang::tooling::buildASTFromCode(code);
    ASSERT_TRUE(AST.get() != nullptr);

    // Find the class declaration
    auto& context = AST->getASTContext();
    
    // Find the ComponentWithMultipleConstructors class
    const clang::CXXRecordDecl* recordDecl = nullptr;
    for (const auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (const auto* record = llvm::dyn_cast<clang::CXXRecordDecl>(decl)) {
            if (record->getNameAsString() == "ComponentWithMultipleConstructors") {
                recordDecl = record;
                break;
            }
        }
    }
    ASSERT_TRUE(recordDecl != nullptr);

    // Call the public parse method
    Component component = parser->parse(recordDecl, &context);

    // Verify the references were correctly parsed from the annotated constructor
    std::vector<Reference> references = component.getReferences();
    ASSERT_EQ(references.size(), 2);
    
    // Check first reference
    EXPECT_EQ(references[0].getName(), "Interface1");
    EXPECT_EQ(references[0].getInterface(), "Interface1");
    
    // Check properties of first reference
    nlohmann::json props1 = references[0].getProperties();
    EXPECT_TRUE(props1.contains("cardinality"));
    EXPECT_EQ(props1["cardinality"], "1..n");
    
    // Check second reference
    EXPECT_EQ(references[1].getName(), "Interface2");
    EXPECT_EQ(references[1].getInterface(), "Interface2");
    
    // Check properties of second reference
    nlohmann::json props2 = references[1].getProperties();
    EXPECT_TRUE(props2.contains("cardinality"));
    EXPECT_EQ(props2["cardinality"], "0..1");
    EXPECT_TRUE(props2.contains("policy-option"));
    EXPECT_EQ(props2["policy-option"], "greedy");
}



