#pragma once
#include "Component.h"
#include "nlohmann/json.hpp"
#include "ManifestMerger.h"
#include <vector>

using json = nlohmann::json;


class JsonGenerator {
public:
    void generateManifest(const std::vector<Component>& components, 
        const std::string& existingPath,
        const std::string& outputPath);

private:
    nlohmann::json generateJson(const std::vector<Component>& components);
    ManifestMerger merger;
};
