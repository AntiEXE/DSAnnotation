#include "ReferenceParser.h"
#include <regex>

Reference ReferenceParser::parse(const std::string& referenceString, const std::string& paramName, const std::string& paramType) {
    Reference reference(paramName, paramType);
    
    // Try to match @reference <InterfaceType> {...} using the simplified interface name
    std::string escapedParamName = std::regex_replace(paramName, std::regex("::"), "\\\\::\\\\");
    std::regex interfaceRegex("@reference\\s+" + escapedParamName + "\\s*\\{([^}]*)\\}");
    std::smatch match;
    
    if (std::regex_search(referenceString, match, interfaceRegex) && match.size() > 1) {
        std::string propString = match[1].str();
        reference.setProperties(propertyParser.parse(propString));
    }
    // Also try with the full paramType in case someone uses the full qualified name
    else {
        std::string escapedParamType = std::regex_replace(paramType, std::regex("::"), "\\\\::\\\\");
        std::regex fullTypeRegex("@reference\\s+" + escapedParamType + "\\s*\\{([^}]*)\\}");
        if (std::regex_search(referenceString, match, fullTypeRegex) && match.size() > 1) {
            std::string propString = match[1].str();
            reference.setProperties(propertyParser.parse(propString));
        }
    }
    
    return reference;
}
