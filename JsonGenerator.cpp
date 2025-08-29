#include "JsonGenerator.h"
#include <fstream>

nlohmann::json JsonGenerator::generateJson(const std::vector<Component>& components) {

    // manifest["bundle.symbolic_name"] = "DSGraph05";
    // manifest["bundle.name"] = "DSGraph05";
    nlohmann::json manifest;
    nlohmann::json scr;
    scr["version"] = 1;
    scr["components"] = nlohmann::json::array();

    for (const auto& component : components) {
        nlohmann::json componentJson;
        componentJson["implementation-class"] = component.getClassName();
        
        // Handle component-level attributes
        if (component.hasAttributes()) {
            nlohmann::json attrs = component.getAttributes();
            
            // Extract service attributes and create service object
            if (attrs.contains("service") && attrs["service"].is_object()) {
                nlohmann::json service;
                
                // Add interfaces to service if they exist
                if (!component.getInterfaces().empty()) {
                    service["interfaces"] = component.getInterfaces();
                }
                
                // Add service attributes
                for (auto& [key, value] : attrs["service"].items()) {
                    service[key] = value;
                }
                
                componentJson["service"] = service;
                attrs.erase("service"); // Remove from component-level attributes
            }
            // If no service attributes but we have interfaces, create service object
            else if (!component.getInterfaces().empty()) {
                nlohmann::json service;
                service["interfaces"] = component.getInterfaces();
                componentJson["service"] = service;
            }
            
            // Add remaining component-level attributes
            for (auto& [key, value] : attrs.items()) {
                componentJson[key] = value;
            }
        }
        // If no attributes but we have interfaces, still create service object
        else if (!component.getInterfaces().empty()) {
            nlohmann::json service;
            service["interfaces"] = component.getInterfaces();
            componentJson["service"] = service;
        }

        // Handle properties section (separate from component attributes)
        if (!component.getProperties().empty()) {
            componentJson["properties"] = component.getProperties();
        }

        // Handle references (unchanged)
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