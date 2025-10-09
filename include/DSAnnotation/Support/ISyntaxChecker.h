#pragma once

#include <string>
#include <string_view>

#include "DSAnnotation/Core/ErrorCollector.h"
#include "DSAnnotation/Support/SourceLocationInfo.h"

namespace dsannotation::support {

class ISyntaxChecker {
public:
    virtual ~ISyntaxChecker() = default;
    virtual bool checkBalancedBraces(const std::string& text,
                                     core::ErrorCollector& errorCollector,
                                     std::string_view contextLabel,
                                     const SourceLocationInfo& location = SourceLocationInfo{}) const = 0;
};

} // namespace dsannotation::support
