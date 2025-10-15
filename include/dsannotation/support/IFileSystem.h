#pragma once

#include <optional>
#include <string>

#include "nlohmann/json.hpp"

namespace dsannotation::support {

class IFileSystem {
public:
    virtual ~IFileSystem() = default;

    virtual bool exists(const std::string& path) const = 0;
    virtual std::optional<std::string> readTextFile(const std::string& path) const = 0;
    virtual std::optional<nlohmann::json> readJsonFile(const std::string& path) const = 0;
    virtual bool writeTextFile(const std::string& path, const std::string& contents) const = 0;
};

} // namespace dsannotation::support
