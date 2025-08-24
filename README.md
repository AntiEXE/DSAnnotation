
# Annotation Parser for CppMicroservices

This tool parses C++ annotations for CppMicroservices, generating a JSON manifest for Declarative Services.

## Compilation

To compile the tool, use the following command:
```bash
clang++ main.cpp ASTVisitor.cpp ComponentParser.cpp Reference.cpp Component.cpp JsonGenerator.cpp PropertyParser.cpp ReferenceParser.cpp ErrorCollector.cpp ErrorReporter.cpp SyntaxChecker.cpp -fPIC -o parser -L$(llvm-config --libdir) -I$(llvm-config --includedir) $(llvm-config --cxxflags --libs) -O3 -lclangTooling -lclangASTMatchers -lclangFormat -lclangFrontend -lclangDriver -lclangParse -lLLVMMCParser -lclangSerialization -lclangSema -lclangEdit -lclangAnalysis -lLLVMBitReader -lLLVMProfileData -lclangToolingCore -lclangAST -lclangRewrite -lclangLex -lclangBasic -lLLVMCore -lLLVMBinaryFormat -lLLVMMC -lLLVMOption -lLLVMSupport -lz -lcurses -lLLVMDemangle
```
## Usage
Run the tool on your C++ files:
```bash
./parser path/to/your/cpp/file.cpp
```

## Class Diagram

```ascii
                                                                                                                           ┌──────────────────────────────────────────────────────┐                                                                                                                                                    
                                                                                                                           │ASTVisitor                                            │                                                                                                                                                    
┌──────────────────────────────────────────────────┐                                                                       ├──────────────────────────────────────────────────────┤                                                                                                                                                    
│JsonGenerator                                     │                                                                       │-Context: ASTContext*                                 │                                                                                                                                                    
├──────────────────────────────────────────────────┤                                                                       │-componentParser: ComponentParser                     │                                                                                                                                                    
│+generateJson(components: vector<Component>): json│                                                                       │-Components: vector<Component>                        │                                                                                                                                                    
└──────────────────────────────────────────────────┘                                                                       │+VisitCXXRecordDecl(Declaration: CXXRecordDecl*): bool│                                                                                                                                                    
                          |                                                                                                │+GetComponents(): vector<Component>                   │                                                                                                                                                    
                          |                                                                                                └──────────────────────────────────────────────────────┘                                                                                                                                                    
                          |                                                                                                                                                                                                                                                                                                            
                          |                                                                                                                                                                                                                                                                                                            
       ┌───────────────────────────────────┐                                                                                                                                                                                                                                                                                           
       │Component                          │                                                                                                                                                                                                                                                                                           
       ├───────────────────────────────────┤                                   ┌──────────────────────────────────────────────────────────────────────────────────────────┐                                                                                                                                                            
       │-className: string                 │                                   │ComponentParser                                                                           │                                                                                                                                                            
       │-interfaces: vector<string>        │                                   ├──────────────────────────────────────────────────────────────────────────────────────────┤         ┌────────────────────────────────┐   ┌────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
       │-properties: json                  │                                   │-errorCollector: ErrorCollector&                                                          │         │ErrorReporter                   │   │SyntaxChecker                                                                                               │
       │-references: vector<Reference>     │                                   │-referenceParser: ReferenceParser                                                         │         ├────────────────────────────────┤   ├────────────────────────────────────────────────────────────────────────────────────────────────────────────┤
       │+addInterface(interface: string)   │                                   │-propertyParser: PropertyParser                                                           │         │-errorCollector: ErrorCollector&│   │+{static} checkBalancedBraces(text: string, errorCollector: ErrorCollector&, loc: SourceLocation): bool     │
       │+setProperties(props: json)        │                                   │+parse(Declaration: CXXRecordDecl*, Context: ASTContext*): Component                      │         │+generateSummary(): string      │   │+{static} checkBalancedParentheses(text: string, errorCollector: ErrorCollector&, loc: SourceLocation): bool│
       │+addReference(ref: Reference)      │                                   │-parseReferences(component: Component&, Declaration: CXXRecordDecl*, Context: ASTContext*)│         │+printReport()                  │   │+{static} checkValidPropertyFormat(text: string, errorCollector: ErrorCollector&, loc: SourceLocation): bool│
       │+getClassName(): string            │                                   │-parseProperties(component: Component&, RC: RawComment*, Context: ASTContext*)            │         └────────────────────────────────┘   └────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
       │+getInterfaces(): vector<string>   │                                   │-parseExternalProperties(component: Component&, filePath: string, Context: ASTContext*)   │                                                                                                                                                            
       │+getProperties(): json             │                                   └──────────────────────────────────────────────────────────────────────────────────────────┘                                                                                                                                                            
       │+getReferences(): vector<Reference>│                                                                                                                                                                                                                                                                                           
       └───────────────────────────────────┘                                                                                                                                                                                                                                                                                           
                          |                                                                                                                                                                                                                                                                                                            
           ┌───────────────────────────┐                                                                                                                                                                                                                                                                                               
           │Reference                  │                                                                                                     ┌──────────────────────────────────────────────────────────────────────────────────────────────────────┐                                                                                  
           ├───────────────────────────┤                                                                                                     │ErrorCollector                                                                                        │                                                                                  
           │-name: string              │   ┌────────────────────────────────────────────────────────────────────────────────┐                ├──────────────────────────────────────────────────────────────────────────────────────────────────────┤                                                                                  
           │-interface: string         │   │ReferenceParser                                                                 │                │-errors: vector<Error>                                                                                │                                                                                  
           │-properties: json          │   ├────────────────────────────────────────────────────────────────────────────────┤                │-SM: SourceManager*                                                                                   │                                                                                  
           │+setProperties(props: json)│   │-propertyParser: PropertyParser                                                 │                │+addError(message: string, location: SourceLocation, severity: ErrorSeverity, category: ErrorCategory)│                                                                                  
           │+getName(): string         │   │+parse(referenceString: string, paramName: string, paramType: string): Reference│                │+getErrors(): vector<Error>                                                                           │                                                                                  
           │+getInterface(): string    │   └────────────────────────────────────────────────────────────────────────────────┘                │+getSourceManager(): SourceManager&                                                                   │                                                                                  
           │+getProperties(): json     │                                                                                                     └──────────────────────────────────────────────────────────────────────────────────────────────────────┘                                                                                  
           └───────────────────────────┘                                                                                                                                                                                                                                                                                               
                                                                                                                                                                                                                                                                                                                                       
                                                                                                                                                                                                                                                                                                                                       
                                                                                        ┌────────────────────────────────────┐                                                                                                                                                                                                         
                                                                                        │PropertyParser                      │                                                                                                                                                                                                         
                                                                                        ├────────────────────────────────────┤                                                                                                                                                                                                         
                                                                                        │+parse(propertiesText: string): json│                                                                                                                                                                                                         
                                                                                        └────────────────────────────────────┘
```

## Example
### Input:
```cpp
#include <memory>
namespace util
{
    class ServiceProvider
    {
    public:
    };

    namespace json
    {

        class JsonSerializer
        {
        public:
            JsonSerializer();
        };
        /** @component
         * @property{./aw.json}
         */
        class JsonSerializerImpl : public util::json::JsonSerializer
        {
        public:
            bool ret()
            {
                return 0;
            }
        };
        /** @component {inject-references = true}
         * @properties {"property1":"value1","property2":["value2a","value2b"],"property3":true}
         */
        class JsonSerializerServiceProvider : public util::ServiceProvider, public util::json::JsonSerializer
        {
        public:
            /**
             * @reference bar
             * @reference foo {policy-option = greedy, cardinality = 1..n}
             */
            JsonSerializerServiceProvider(const std::shared_ptr<util::json::JsonSerializer> &bar, const std::shared_ptr<util::json::JsonSerializerImpl> &foo);

            bool ret()
            {
                return 0;
            }
        };
    }
}
```
### Output
```json
{
    "scr": {
        "components": [
            {
                "implementation-class": "util::json::JsonSerializerImpl",
                "properties": {
                    "property1": "value1",
                    "property2": [
                        "value2a",
                        "value2b"
                    ],
                    "property3": true
                },
                "service": {
                    "interfaces": [
                        "util::json::JsonSerializer"
                    ]
                }
            },
            {
                "implementation-class": "util::json::JsonSerializerServiceProvider",
                "properties": {
                    "property1": "value1",
                    "property2": [
                        "value2a",
                        "value2b"
                    ],
                    "property3": true
                },
                "references": [
                    {
                        "interface": "util::json::JsonSerializer",
                        "name": "bar"
                    },
                    {
                        "cardinality": "1..n",
                        "interface": "util::json::JsonSerializerImpl",
                        "name": "foo",
                        "policy-option": "greedy"
                    }
                ],
                "service": {
                    "interfaces": [
                        "util::ServiceProvider",
                        "util::json::JsonSerializer"
                    ]
                }
            }
        ],
        "version": 1
    }
}
```
## Features
-   Parses @component, @reference, and @properties annotations
-   Supports loading properties from external JSON files
-   Provides detailed error reporting

