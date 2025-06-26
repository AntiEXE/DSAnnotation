#include "ManifestMerger.h"
#include <fstream>

nlohmann::json ManifestMerger::merge(const std::string& existingPath, const nlohmann::json& generated) {
    nlohmann::json existing = readExistingManifest(existingPath);
    if (existing.empty()) {
        return generated;
    }

    // Preserve non-SCR fields from existing manifest
    nlohmann::json result = existing;
    
    // Ensure SCR section exists
    if (!result.contains("scr")) {
        result["scr"] = nlohmann::json::object();
    }
    
    // Merge components with priority to generated content
    mergeComponents(result["scr"], generated["scr"]);
    
    return result;
}

nlohmann::json ManifestMerger::readExistingManifest(const std::string& path) {
    try {
        std::ifstream file(path);
        if (file.is_open()) {
            return nlohmann::json::parse(file);
        }
    } catch (...) {
        // Return empty JSON if file doesn't exist or is invalid
    }
    return nlohmann::json::object();
}

void ManifestMerger::mergeComponents(nlohmann::json& target, const nlohmann::json& source) {
    if (!source.contains("components")) {
        return;
    }

    if (!target.contains("components")) {
        target["components"] = nlohmann::json::array();
    }

    // Update or add components from source
    for (const auto& sourceComponent : source["components"]) {
        bool found = false;
        for (auto& targetComponent : target["components"]) {
            if (targetComponent["implementation-class"] == sourceComponent["implementation-class"]) {
                targetComponent.update(sourceComponent);
                found = true;
                break;
            }
        }
        if (!found) {
            target["components"].push_back(sourceComponent);
        }
    }
}
