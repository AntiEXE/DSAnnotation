#include "DSAnnotation/Core/Reference.h"

namespace dsannotation::core {

Reference::Reference(std::string name, std::string interface)
    : name_(std::move(name)), interface_(std::move(interface)), properties_(nlohmann::json::object()) {}

void Reference::setProperties(nlohmann::json properties) {
    properties_ = std::move(properties);
}

} // namespace dsannotation::core
