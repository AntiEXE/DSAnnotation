#pragma once
#include "nlohmann/json.hpp"
#include <string>

using json = nlohmann::json;

class PropertyParser {
public:
    nlohmann::json parse(const std::string& propertyString);
};
