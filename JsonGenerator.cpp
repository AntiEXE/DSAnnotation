#include "JsonGenerator.h"
#include <fstream>

nlohmann::json JsonGenerator::generateJson(const std::vector<Component>& components) {
    nlohmann::json manifest;
    // manifest["bundle.symbolic_name"] = "DSGraph05";
    // manifest["bundle.name"] = "DSGraph05";
    
    nlohmann::json scr;
    scr["version"] = 1;
    scr["components"] = nlohmann::json::array();

    for (const auto& component : components) {
        nlohmann::json componentJson;
        componentJson["implementation-class"] = component.getClassName();
        
        if (!component.getInterfaces().empty()) {
            nlohmann::json service;
            service["interfaces"] = component.getInterfaces();
            componentJson["service"] = service;
        }

        if (!component.getProperties().empty()) {
            componentJson["properties"] = component.getProperties();
        }

        if (!component.getReferences().empty()) {
            nlohmann::json references = nlohmann::json::array();
            for (const auto& ref : component.getReferences()) {
                nlohmann::json refJson;
                refJson["name"] = ref.getName();
                refJson["interface"] = ref.getInterface();
                if (!ref.getProperties().empty()) {
                    refJson.update(ref.getProperties());
                }
                references.push_back(refJson);
            }
            componentJson["references"] = references;
        }

        scr["components"].push_back(componentJson);
    }

    manifest["scr"] = scr;
    return manifest;
}

void JsonGenerator::generateManifest(const std::vector<Component>& components,
    const std::string& existingPath,
    const std::string& outputPath) {
nlohmann::json generated = generateJson(components);
nlohmann::json final = merger.merge(existingPath, generated);

std::ofstream outFile(outputPath);
outFile << final.dump(4);
outFile.close();
}