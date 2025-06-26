#pragma once
#include "nlohmann/json.hpp"
#include <string>

using json = nlohmann::json;

class Reference {
public:
    Reference(const std::string& name, const std::string& interface);
    void setProperties(const nlohmann::json& props);

    std::string getName() const;
    std::string getInterface() const;
    nlohmann::json getProperties() const;

private:
    std::string name;
    std::string interface;
    nlohmann::json properties;
};