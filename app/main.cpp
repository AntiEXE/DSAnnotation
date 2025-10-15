#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include "dsannotation/config/ParserConfig.h"
#include "dsannotation/core/ErrorCollector.h"
#include "dsannotation/parsing/ASTVisitor.h"
#include "dsannotation/parsing/ComponentParser.h"
#include "dsannotation/parsing/PropertyParser.h"
#include "dsannotation/parsing/ReferenceParser.h"
#include "dsannotation/serialization/JsonManifestBuilder.h"
#include "dsannotation/serialization/JsonManifestWriter.h"
#include "dsannotation/serialization/ManifestMerger.h"
#include "dsannotation/support/ErrorReporter.h"
#include "dsannotation/support/LocalFileSystem.h"

#include <memory>
#include <utility>

using namespace clang::tooling;
using namespace llvm;

namespace dsannotation::app {

static cl::OptionCategory ToolCategory("ds-annotation parser options");

static cl::opt<std::string> InputManifest(
    "i",
    cl::desc("Specify existing manifest.json to merge with"),
    cl::value_desc("file"),
    cl::cat(ToolCategory),
    cl::Optional);

static cl::opt<std::string> OutputDir(
    "o",
    cl::desc("Specify output directory for manifest.json"),
    cl::value_desc("directory"),
    cl::cat(ToolCategory),
    cl::Optional);

class ComponentASTConsumer : public clang::ASTConsumer {
public:
    explicit ComponentASTConsumer(config::ParserConfig config)
        : config_(std::move(config)) {}

    void HandleTranslationUnit(clang::ASTContext& context) override {
        support::LocalFileSystem fileSystem;
        parsing::PropertyParser propertyParser;
        parsing::ReferenceParser referenceParser(propertyParser);
        core::ErrorCollector errorCollector(context.getSourceManager());

        parsing::ComponentParser componentParser(propertyParser,
                                                 referenceParser,
                                                 fileSystem,
                                                 errorCollector,
                                                 config_);

        parsing::ASTVisitor visitor(context, componentParser);
        visitor.TraverseDecl(context.getTranslationUnitDecl());

        serialization::JsonManifestBuilder manifestBuilder;
        serialization::ManifestMerger manifestMerger(fileSystem);
        const int indentation = config_.compactJson ? -1 : config_.jsonIndentation;
        serialization::JsonManifestWriter manifestWriter(manifestBuilder,
                                                          manifestMerger,
                                                          fileSystem,
                                                          indentation);

        auto manifestResult = manifestWriter.writeManifest(visitor.components(),
                                                           config_.inputManifestPath.value_or(""),
                                                           config_.outputPath());
        if (manifestResult.hasError()) {
            errorCollector.addError(manifestResult.error(),
                                    core::ErrorSeverity::Error,
                                    core::ErrorCategory::General);
        }

        support::ErrorReporter reporter(errorCollector);
        if (config_.verboseOutput || !errorCollector.errors().empty()) {
            reporter.print();
        }
    }

private:
    config::ParserConfig config_;
};

class ComponentAction : public clang::ASTFrontendAction {
public:
    explicit ComponentAction(config::ParserConfig config)
        : config_(std::move(config)) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& CI,
                                                          llvm::StringRef) override {
        return std::make_unique<ComponentASTConsumer>(config_);
    }

private:
    config::ParserConfig config_;
};

class ComponentActionFactory : public clang::tooling::FrontendActionFactory {
public:
    explicit ComponentActionFactory(config::ParserConfig config)
        : config_(std::move(config)) {}

    std::unique_ptr<clang::FrontendAction> create() override {
        return std::make_unique<ComponentAction>(config_);
    }

private:
    config::ParserConfig config_;
};

} // namespace dsannotation::app

int main(int argc, const char** argv) {
    auto expectedParser = CommonOptionsParser::create(argc, argv, dsannotation::app::ToolCategory);
    if (!expectedParser) {
        llvm::errs() << expectedParser.takeError();
        return 1;
    }

    CommonOptionsParser& optionsParser = expectedParser.get();
    ClangTool tool(optionsParser.getCompilations(), optionsParser.getSourcePathList());

    dsannotation::config::ParserConfig config;
    if (!dsannotation::app::OutputDir.getValue().empty()) {
        config.outputDirectory = dsannotation::app::OutputDir.getValue();
    }
    if (!dsannotation::app::InputManifest.getValue().empty()) {
        config.inputManifestPath = dsannotation::app::InputManifest.getValue();
    }

    auto factory = std::make_unique<dsannotation::app::ComponentActionFactory>(config);
    return tool.run(factory.get());
}
