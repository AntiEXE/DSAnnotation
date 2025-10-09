#include "DSAnnotation/Parsing/ReferenceParser.h"

#include <cctype>
#include <regex>

namespace dsannotation::parsing {

namespace {
std::string escapeForRegex(std::string_view text) {
    std::string escaped;
    escaped.reserve(text.size() * 2);
    for (char c : text) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            escaped.push_back(c);
        } else {
            escaped.push_back('\\');
            escaped.push_back(c);
        }
    }
    return escaped;
}
}

ReferenceParser::ReferenceParser(const IPropertyParser& propertyParser)
    : propertyParser_(propertyParser) {}

core::Reference ReferenceParser::parse(std::string_view referenceAnnotations,
                                       std::string_view parameterName,
                                       std::string_view parameterQualifiedType) const {
    core::Reference reference{std::string(parameterName), std::string(parameterQualifiedType)};

    const std::regex namePattern(std::string("@reference\\s+") + escapeForRegex(parameterName) + "\\s*\\{([^}]*)\\}");
    std::match_results<std::string_view::const_iterator> match;
    if (std::regex_search(referenceAnnotations.begin(), referenceAnnotations.end(), match, namePattern) && match.size() > 1) {
        auto properties = propertyParser_.parse(match[1].str());
        reference.setProperties(std::move(properties));
        return reference;
    }

    const std::regex typePattern(std::string("@reference\\s+") + escapeForRegex(parameterQualifiedType) + "\\s*\\{([^}]*)\\}");
    if (std::regex_search(referenceAnnotations.begin(), referenceAnnotations.end(), match, typePattern) && match.size() > 1) {
        auto properties = propertyParser_.parse(match[1].str());
        reference.setProperties(std::move(properties));
    }

    return reference;
}

} // namespace dsannotation::parsing
