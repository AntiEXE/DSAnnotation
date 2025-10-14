#include "dsannotation/support/LocalFileSystem.h"

#include <fstream>
#include <sstream>

namespace dsannotation::support {

bool LocalFileSystem::exists(const std::string& path) const {
    return std::filesystem::exists(path);
}

std::optional<std::string> LocalFileSystem::readTextFile(const std::string& path) const {
    std::ifstream input(path);
    if (!input.is_open()) {
        return std::nullopt;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::optional<nlohmann::json> LocalFileSystem::readJsonFile(const std::string& path) const {
    std::ifstream input(path);
    if (!input.is_open()) {
        return std::nullopt;
    }
    nlohmann::json json = nlohmann::json::parse(input, nullptr, false);
    if (json.is_discarded()) {
        return std::nullopt;
    }
    return json;
}

bool LocalFileSystem::writeTextFile(const std::string& path, const std::string& contents) const {
    auto parent = std::filesystem::path(path).parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    std::ofstream output(path);
    if (!output.is_open()) {
        return false;
    }

    output << contents;
    return static_cast<bool>(output);
}

} // namespace dsannotation::support
