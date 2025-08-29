#pragma once
#include "nlohmann/json.hpp"
#include <string>

using json = nlohmann::json;

class PropertyParser {
public:
    nlohmann::json parse(const std::string& propertyString);
private:
    nlohmann::json parseValue(const std::string& value);
    void setNestedProperty(nlohmann::json& json, const std::string& key, const nlohmann::json& value);
};
