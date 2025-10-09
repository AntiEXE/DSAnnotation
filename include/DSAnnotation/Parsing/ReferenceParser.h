#pragma once

#include <regex>
#include <string>

#include "DSAnnotation/Parsing/IReferenceParser.h"
#include "DSAnnotation/Parsing/IPropertyParser.h"

namespace dsannotation::parsing {

class ReferenceParser final : public IReferenceParser {
public:
    explicit ReferenceParser(const IPropertyParser& propertyParser);

    core::Reference parse(std::string_view referenceAnnotations,
                          std::string_view parameterName,
                          std::string_view parameterQualifiedType) const override;

private:
    const IPropertyParser& propertyParser_;
};

} // namespace dsannotation::parsing
