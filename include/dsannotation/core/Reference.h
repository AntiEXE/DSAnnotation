#pragma once

#include <string>
#include <utility>
#include <vector>
#include "nlohmann/json.hpp"

namespace dsannotation::core {

class Reference {
public:
    Reference(std::string name, std::string interface);

    void setProperties(nlohmann::json properties);

    const std::string& name() const noexcept { return name_; }
    const std::string& interface() const noexcept { return interface_; }
    const nlohmann::json& properties() const noexcept { return properties_; }
    bool hasProperties() const noexcept { return !properties_.empty(); }

private:
    std::string name_;
    std::string interface_;
    nlohmann::json properties_;
};

using ReferenceList = std::vector<Reference>;

} // namespace dsannotation::core
