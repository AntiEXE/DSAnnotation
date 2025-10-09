#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace dsannotation::config {

struct ParserConfig {
    std::string outputDirectory{"."};
    std::optional<std::string> inputManifestPath{};
    std::string outputFileName{"manifest.json"};
    bool verboseOutput{true};
    bool strictMode{false};

    // Validation settings
    bool validateSyntax{true};
    bool validateReferences{true};

    // JSON formatting
    int jsonIndentation{4};
    bool compactJson{true};

    bool isValid() const {
        return !outputDirectory.empty() && !outputFileName.empty();
    }

    std::string outputPath() const {
        std::filesystem::path dir(outputDirectory);
        std::filesystem::path file(outputFileName);
        return (dir / file).string();
    }
};

} // namespace dsannotation::config
