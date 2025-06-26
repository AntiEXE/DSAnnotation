#pragma once
#include "nlohmann/json.hpp"
#include <string>

class ManifestMerger {
public:
    nlohmann::json merge(const std::string& existingPath, const nlohmann::json& generated);

private:
    nlohmann::json readExistingManifest(const std::string& path);
    void mergeComponents(nlohmann::json& target, const nlohmann::json& source);
};
