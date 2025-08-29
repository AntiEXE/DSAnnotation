#include "PropertyParser.h"
#include <regex>
#include <sstream>

nlohmann::json PropertyParser::parseValue(const std::string& value) {
    std::string trimmedValue = std::regex_replace(value, std::regex("^\\s+|\\s+$"), "");
    
    if (!trimmedValue.empty() && trimmedValue.front() == '[' && trimmedValue.back() == ']') {
        // Array handling
        std::string arrayContent = trimmedValue.substr(1, trimmedValue.length() - 2);
        std::vector<std::string> arrayValues;
        std::istringstream arrayStream(arrayContent);
        std::string arrayItem;
        while (std::getline(arrayStream, arrayItem, ',')) {
            arrayItem = std::regex_replace(arrayItem, std::regex("^\\s+|\\s+$"), "");
            arrayValues.push_back(arrayItem);
        }
        return arrayValues;
    } else if (trimmedValue == "true") {
        return true;
    } else if (trimmedValue == "false") {
        return false;
    } else {
        return trimmedValue;
    }
}

void PropertyParser::setNestedProperty(nlohmann::json& json, const std::string& key, const nlohmann::json& value) {
    size_t dotPos = key.find('.');
    if (dotPos != std::string::npos) {
        std::string parentKey = key.substr(0, dotPos);
        std::string childKey = key.substr(dotPos + 1);
        
        // Create nested object if it doesn't exist
        if (!json.contains(parentKey)) {
            json[parentKey] = nlohmann::json::object();
        }
        
        json[parentKey][childKey] = value;
    } else {
        json[key] = value;
    }
}

nlohmann::json PropertyParser::parse(const std::string& propertiesText) {
    nlohmann::json propertiesJson;
    if(propertiesText.empty()) return propertiesJson;

    std::vector<std::string> properties;
    std::string temp;
    bool inArray = false;

    // Split properties by comma (respecting arrays)
    for (char c : propertiesText) {
        if (c == '[') inArray = true;
        if (c == ']') inArray = false;
        if (c == ',' && !inArray) {
            properties.push_back(temp);
            temp.clear();
        } else {
            temp += c;
        }
    }
    if (!temp.empty()) {
        properties.push_back(temp);
    }
    
    // Process each property
    for (const auto& property : properties) {
        size_t delimiterPos = property.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = property.substr(0, delimiterPos);
            std::string value = property.substr(delimiterPos + 1);

            key = std::regex_replace(key, std::regex("^\\s+|\\s+$"), "");
            
            // Parse value with type identification
            nlohmann::json parsedValue = parseValue(value);
            
            // Set property (handles dot notation)
            setNestedProperty(propertiesJson, key, parsedValue);
        }
    }
    return propertiesJson;
}