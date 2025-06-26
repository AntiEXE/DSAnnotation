#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/CompilerInstance.h"
#include "ASTVisitor.h"
#include "JsonGenerator.h"
#include "ErrorCollector.h"
#include "ErrorReporter.h"
#include <fstream>
#include <filesystem>

using namespace clang::tooling;
using namespace llvm;

static llvm::cl::OptionCategory MyToolCategory("my-tool options");

static llvm::cl::opt<std::string> InputManifest(
    "i",
    llvm::cl::desc("Specify existing manifest.json to merge with"),
    llvm::cl::value_desc("file"),
    llvm::cl::cat(MyToolCategory),
    llvm::cl::Optional
);

static llvm::cl::opt<std::string> OutputDir(
    "o",
    llvm::cl::desc("Specify output directory for manifest.json"),
    llvm::cl::value_desc("directory"),
    llvm::cl::cat(MyToolCategory),
    llvm::cl::Optional);
class ComponentASTConsumer : public clang::ASTConsumer {
public:
    explicit ComponentASTConsumer(clang::ASTContext *Context) 
    : errorCollector(&Context->getSourceManager()),
      Visitor(Context, errorCollector) {}
    void HandleTranslationUnit(clang::ASTContext &Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
        std::string outputPath = "manifest.json";
        if (!OutputDir.empty()) {
            std::filesystem::path dirPath(OutputDir.getValue());
            std::filesystem::create_directories(dirPath);
            outputPath = (dirPath / "manifest.json").string();
        }
        JsonGenerator jsonGen;
        jsonGen.generateManifest(
            Visitor.GetComponents(),
            InputManifest.getValue(),  // input path
            outputPath                // output path
        );
        // nlohmann::json manifest = jsonGen.generateJson(Visitor.GetComponents());
                
        // std::ofstream outFile(outputPath);
        // outFile << manifest.dump(4);
        // outFile.close();
        ErrorReporter reporter(errorCollector);
        reporter.printReport();
    }

private:
    ASTVisitor Visitor;
    ErrorCollector errorCollector;
};

class ComponentAction : public clang::ASTFrontendAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, StringRef file) override {
        return std::make_unique<ComponentASTConsumer>(&CI.getASTContext());
    }
};

int main(int argc, const char **argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    CommonOptionsParser& OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

    return Tool.run(newFrontendActionFactory<ComponentAction>().get());
}
