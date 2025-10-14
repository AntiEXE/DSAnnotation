#pragma once

#include <string_view>

#include "dsannotation/core/Reference.h"

namespace dsannotation::parsing {

class IReferenceParser {
public:
    virtual ~IReferenceParser() = default;
    virtual core::Reference parse(std::string_view referenceAnnotations,
                                  std::string_view parameterName,
                                  std::string_view parameterQualifiedType) const = 0;
};

} // namespace dsannotation::parsing
