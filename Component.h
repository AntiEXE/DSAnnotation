#pragma once
#include "Reference.h"
#include "nlohmann/json.hpp"
#include <string>
#include <vector>

using json = nlohmann::json;


class Component {
public:
    Component(const std::string& className);
    void addInterface(const std::string& interface);
    void setProperties(const nlohmann::json& props);
    void setAttributes(const nlohmann::json& attrs);
    void addReference(const Reference& ref);

    bool hasAttributes() const;
    nlohmann::json getAttributes() const;
    std::string getClassName() const;
    std::vector<std::string> getInterfaces() const;
    nlohmann::json getProperties() const;
    std::vector<Reference> getReferences() const;

private:
    std::string className;
    std::vector<std::string> interfaces;
    nlohmann::json attributes;
    nlohmann::json properties;
    std::vector<Reference> references;
};