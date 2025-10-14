#pragma once

#include "dsannotation/parsing/IPropertyParser.h"

namespace dsannotation::parsing {

class PropertyParser final : public IPropertyParser {
public:
    nlohmann::json parse(std::string_view propertiesText) const override;

private:
    nlohmann::json parseValue(std::string_view value) const;
    void setNestedProperty(nlohmann::json& json,
                           std::string_view key,
                           const nlohmann::json& value) const;
};

} // namespace dsannotation::parsing
