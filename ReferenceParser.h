#pragma once
#include "Reference.h"
#include "PropertyParser.h"
#include <string>

class ReferenceParser {
public:
    // Parses @reference annotations. Now supports both:
    // @reference InterfaceName {properties...} (preferred)
    // @reference fully::qualified::InterfaceName {properties...}
    Reference parse(const std::string& referenceString, const std::string& paramName, const std::string& paramType);
private:
    PropertyParser propertyParser;
};
