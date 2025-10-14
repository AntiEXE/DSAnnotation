#pragma once

#include <string>
#include <vector>

#include "nlohmann/json.hpp"
#include "dsannotation/core/Reference.h"

namespace dsannotation::core {

class Component {
public:
    explicit Component(std::string className);

    void addInterface(std::string interfaceName);
    void setProperties(nlohmann::json properties);
    void setAttributes(nlohmann::json attributes);
    void addReference(Reference reference);

    const std::string& className() const noexcept { return className_; }
    const std::vector<std::string>& interfaces() const noexcept { return interfaces_; }
    const nlohmann::json& properties() const noexcept { return properties_; }
    const nlohmann::json& attributes() const noexcept { return attributes_; }
    const std::vector<Reference>& references() const noexcept { return references_; }

    bool hasAttributes() const noexcept;
    bool hasProperties() const noexcept { return !properties_.empty(); }

private:
    std::string className_;
    std::vector<std::string> interfaces_;
    nlohmann::json attributes_;
    nlohmann::json properties_;
    std::vector<Reference> references_;
};

using ComponentList = std::vector<Component>;

} // namespace dsannotation::core
