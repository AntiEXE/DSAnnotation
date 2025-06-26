#include "PropertyParser.h"
#include <regex>

nlohmann::json PropertyParser::parse(const std::string& propertiesText) {
    nlohmann::json propertiesJson;
    if(propertiesText.empty()) return propertiesJson;

    std::vector<std::string> properties;
    std::string temp;
    bool inArray = false;

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
    for (const auto& property : properties) {
        size_t delimiterPos = property.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = property.substr(0, delimiterPos);
            std::string value = property.substr(delimiterPos + 1);

            key = std::regex_replace(key, std::regex("^\\s+|\\s+$"), "");
            value = std::regex_replace(value, std::regex("^\\s+|\\s+$"), "");

            if (!value.empty() && value.front() == '[' && value.back() == ']') {
                value = value.substr(1, value.length() - 2);
                std::vector<std::string> arrayValues;
                std::istringstream arrayStream(value);
                std::string arrayItem;
                while (std::getline(arrayStream, arrayItem, ',')) {
                    arrayItem = std::regex_replace(arrayItem, std::regex("^\\s+|\\s+$"), "");
                    arrayValues.push_back(arrayItem);
                }
                propertiesJson[key] = arrayValues;
            } else {
                if (value == "true") {
                    propertiesJson[key] = true;
                } else if (value == "false") {
                    propertiesJson[key] = false;
                } else {
                    propertiesJson[key] = value;
                }
            }
        }
    }
    return propertiesJson;
}
