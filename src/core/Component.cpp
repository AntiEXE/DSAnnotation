#include "dsannotation/core/Component.h"

#include <utility>

namespace dsannotation::core {

Component::Component(std::string className)
    : className_(std::move(className)),
      attributes_(nlohmann::json::object()),
      properties_(nlohmann::json::object()) {}

void Component::addInterface(std::string interfaceName) {
    interfaces_.push_back(std::move(interfaceName));
}

void Component::setProperties(nlohmann::json properties) {
    properties_ = std::move(properties);
}

void Component::setAttributes(nlohmann::json attributes) {
    attributes_ = std::move(attributes);
}

void Component::addReference(Reference reference) {
    references_.push_back(std::move(reference));
}

bool Component::hasAttributes() const noexcept {
    return !attributes_.empty();
}

} // namespace dsannotation::core
