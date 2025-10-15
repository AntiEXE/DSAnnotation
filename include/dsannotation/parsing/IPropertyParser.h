#pragma once

#include <string_view>

#include "nlohmann/json.hpp"

namespace dsannotation::parsing {

class IPropertyParser {
public:
    virtual ~IPropertyParser() = default;
    virtual nlohmann::json parse(std::string_view propertiesText) const = 0;
};

} // namespace dsannotation::parsing
