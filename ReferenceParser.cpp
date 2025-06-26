#include "ReferenceParser.h"
#include <regex>

Reference ReferenceParser::parse(const std::string& referenceString, const std::string& paramName, const std::string& paramType) {
    Reference reference(paramName, paramType);
    
    std::regex propRegex("@reference\\s+" + paramName + "\\s*\\{([^}]*)\\}");
    std::smatch match;
    if (std::regex_search(referenceString, match, propRegex) && match.size() > 1) {
        std::string propString = match[1].str();
        reference.setProperties(propertyParser.parse(propString));
    }
    
    return reference;
}
