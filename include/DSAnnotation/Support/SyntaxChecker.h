#pragma once

#include "DSAnnotation/Support/ISyntaxChecker.h"
#include "DSAnnotation/Support/SourceLocationInfo.h"

namespace dsannotation::support {

class SyntaxChecker final : public ISyntaxChecker {
public:
    bool checkBalancedBraces(const std::string& text,
                             core::ErrorCollector& errorCollector,
                             std::string_view contextLabel,
                             const SourceLocationInfo& location = SourceLocationInfo{}) const override;
};

} // namespace dsannotation::support
