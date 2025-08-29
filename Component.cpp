#include "Component.h"

Component::Component(const std::string& className) : className(className) {}

void Component::addInterface(const std::string& interface) {
    interfaces.push_back(interface);
}

void Component::setProperties(const nlohmann::json& props) {
    properties = props;
}

void Component::addReference(const Reference& ref) {
    references.push_back(ref);
}

void Component::setAttributes(const nlohmann::json& attrs) {
    attributes = attrs;
}

std::string Component::getClassName() const {
    return className;
}

std::vector<std::string> Component::getInterfaces() const {
    return interfaces;
}

nlohmann::json Component::getProperties() const {
    return properties;
}

std::vector<Reference> Component::getReferences() const {
    return references;
}

// NEW: Get component attributes
nlohmann::json Component::getAttributes() const {
    return attributes;
}

bool Component::hasAttributes() const {
    return !attributes.empty();
}
