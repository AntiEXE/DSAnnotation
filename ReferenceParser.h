#pragma once
#include "Reference.h"
#include "PropertyParser.h"
#include <string>

class ReferenceParser {
public:
    Reference parse(const std::string& referenceString, const std::string& paramName, const std::string& paramType);
private:
    PropertyParser propertyParser;
};
