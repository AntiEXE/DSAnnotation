#include "Reference.h"

Reference::Reference(const std::string& name, const std::string& interface)
    : name(name), interface(interface) {}

void Reference::setProperties(const nlohmann::json& props) {
    properties = props;
}

std::string Reference::getName() const {
    return name;
}

std::string Reference::getInterface() const {
    return interface;
}

nlohmann::json Reference::getProperties() const {
    return properties;
}
