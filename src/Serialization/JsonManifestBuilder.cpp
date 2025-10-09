#include "DSAnnotation/Serialization/JsonManifestBuilder.h"

namespace dsannotation::serialization {

nlohmann::json JsonManifestBuilder::buildManifest(const core::ComponentList& components) const {
    nlohmann::json manifest;
    nlohmann::json scr;
    scr["version"] = 1;
    scr["components"] = nlohmann::json::array();

    for (const auto& component : components) {
        nlohmann::json componentJson;
        componentJson["implementation-class"] = component.className();

        if (component.hasAttributes()) {
            nlohmann::json attributes = component.attributes();
            if (attributes.contains("service") && attributes["service"].is_object()) {
                nlohmann::json service = attributes["service"];
                if (!component.interfaces().empty()) {
                    service["interfaces"] = component.interfaces();
                }
                componentJson["service"] = service;
                attributes.erase("service");
            } else if (!component.interfaces().empty()) {
                nlohmann::json service;
                service["interfaces"] = component.interfaces();
                componentJson["service"] = service;
            }

            for (auto& [key, value] : attributes.items()) {
                componentJson[key] = value;
            }
        } else if (!component.interfaces().empty()) {
            nlohmann::json service;
            service["interfaces"] = component.interfaces();
            componentJson["service"] = service;
        }

        if (component.hasProperties()) {
            componentJson["properties"] = component.properties();
        }

        if (!component.references().empty()) {
            nlohmann::json references = nlohmann::json::array();
            for (const auto& reference : component.references()) {
                nlohmann::json referenceJson;
                referenceJson["name"] = reference.name();
                referenceJson["interface"] = reference.interface();
                if (reference.hasProperties()) {
                    referenceJson.update(reference.properties());
                }
                references.push_back(std::move(referenceJson));
            }
            componentJson["references"] = std::move(references);
        }

        scr["components"].push_back(std::move(componentJson));
    }

    manifest["scr"] = std::move(scr);
    return manifest;
}

} // namespace dsannotation::serialization
