#include "dsannotation/parsing/PropertyParser.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace dsannotation::parsing {

nlohmann::json PropertyParser::parse(std::string_view propertiesText) const {
    nlohmann::json propertiesJson = nlohmann::json::object();
    if (propertiesText.empty()) {
        return propertiesJson;
    }

    std::vector<std::string> properties;
    std::string current;
    bool inArray = false;

    for (char c : propertiesText) {
        if (c == '[') {
            inArray = true;
        } else if (c == ']') {
            inArray = false;
        }

        if (c == ',' && !inArray) {
            if (!current.empty()) {
                properties.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(c);
        }
    }

    if (!current.empty()) {
        properties.push_back(current);
    }

    std::regex trimRegex("^\\s+|\\s+$");
    for (auto& property : properties) {
        property = std::regex_replace(property, trimRegex, "");
        auto delimiterPos = property.find('=');
        if (delimiterPos == std::string::npos) {
            continue;
        }

        std::string key = std::regex_replace(property.substr(0, delimiterPos), trimRegex, "");
        std::string value = std::regex_replace(property.substr(delimiterPos + 1), trimRegex, "");

        auto parsedValue = parseValue(value);
        setNestedProperty(propertiesJson, key, parsedValue);
    }

    return propertiesJson;
}

nlohmann::json PropertyParser::parseValue(std::string_view value) const {
    std::string trimmed = std::regex_replace(std::string(value), std::regex("^\\s+|\\s+$"), "");
    if (trimmed.empty()) {
        return nlohmann::json();
    }

    if (trimmed.front() == '[' && trimmed.back() == ']') {
        std::string content = trimmed.substr(1, trimmed.size() - 2);
        std::vector<std::string> arrayValues;
        std::istringstream stream(content);
        std::string item;
        while (std::getline(stream, item, ',')) {
            arrayValues.push_back(std::regex_replace(item, std::regex("^\\s+|\\s+$"), ""));
        }
        return arrayValues;
    }

    if (trimmed == "true") {
        return true;
    }

    if (trimmed == "false") {
        return false;
    }

    // Check for cardinality patterns like "0..1", "1..1", "0..n", "1..n" etc.
    // These should remain as strings, not be parsed as numbers
    std::regex cardinalityPattern(R"(^\d+\.\.\d*[n]?$)");
    if (std::regex_match(trimmed, cardinalityPattern)) {
        return trimmed;
    }

    if (!trimmed.empty() && std::all_of(trimmed.begin(), trimmed.end(), [](char ch) {
            return std::isdigit(static_cast<unsigned char>(ch)) || ch == '.' || ch == '-';
        })) {
        try {
            if (trimmed.find('.') != std::string::npos) {
                return std::stod(trimmed);
            }
            return std::stoi(trimmed);
        } catch (...) {
            // fallthrough to string
        }
    }

    if (trimmed.size() >= 2 &&
        ((trimmed.front() == '"' && trimmed.back() == '"') ||
         (trimmed.front() == '\'' && trimmed.back() == '\''))) {
        return trimmed.substr(1, trimmed.size() - 2);
    }

    return trimmed;
}

void PropertyParser::setNestedProperty(nlohmann::json& json,
                                       std::string_view key,
                                       const nlohmann::json& value) const {
    auto dotPos = key.find('.');
    if (dotPos != std::string_view::npos) {
        auto parentKey = std::string(key.substr(0, dotPos));
        auto childKey = std::string(key.substr(dotPos + 1));
        auto& child = json[parentKey];
        if (!child.is_object()) {
            child = nlohmann::json::object();
        }
        child[childKey] = value;
    } else {
        json[std::string(key)] = value;
    }
}

} // namespace dsannotation::parsing
