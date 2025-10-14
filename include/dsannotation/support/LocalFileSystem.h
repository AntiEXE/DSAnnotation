#pragma once

#include <filesystem>

#include "dsannotation/support/IFileSystem.h"

namespace dsannotation::support {

class LocalFileSystem final : public IFileSystem {
public:
    bool exists(const std::string& path) const override;
    std::optional<std::string> readTextFile(const std::string& path) const override;
    std::optional<nlohmann::json> readJsonFile(const std::string& path) const override;
    bool writeTextFile(const std::string& path, const std::string& contents) const override;
};

} // namespace dsannotation::support
