#include "dsannotation/serialization/ManifestMerger.h"

namespace dsannotation::serialization {

ManifestMerger::ManifestMerger(const support::IFileSystem& fileSystem)
    : fileSystem_(fileSystem) {}

nlohmann::json ManifestMerger::merge(const std::string& existingPath,
                                     const nlohmann::json& generated) const {
    auto existing = readExistingManifest(existingPath);
    if (existing.empty()) {
        return generated;
    }

    nlohmann::json result = existing;
    if (!result.contains("scr")) {
        result["scr"] = nlohmann::json::object();
    }

    if (!generated.contains("scr")) {
        return result;
    }

    mergeComponents(result["scr"], generated["scr"]);
    return result;
}

nlohmann::json ManifestMerger::readExistingManifest(const std::string& path) const {
    if (path.empty() || !fileSystem_.exists(path)) {
        return nlohmann::json::object();
    }

    auto json = fileSystem_.readJsonFile(path);
    if (!json) {
        return nlohmann::json::object();
    }
    return *json;
}

void ManifestMerger::mergeComponents(nlohmann::json& target, const nlohmann::json& source) const {
    if (!source.contains("components")) {
        return;
    }

    if (!target.contains("components")) {
        target["components"] = nlohmann::json::array();
    }

    for (const auto& sourceComponent : source["components"]) {
        bool updated = false;
        for (auto& targetComponent : target["components"]) {
            if (targetComponent.value("implementation-class", "") ==
                sourceComponent.value("implementation-class", "")) {
                targetComponent.update(sourceComponent);
                updated = true;
                break;
            }
        }
        if (!updated) {
            target["components"].push_back(sourceComponent);
        }
    }
}

} // namespace dsannotation::serialization
