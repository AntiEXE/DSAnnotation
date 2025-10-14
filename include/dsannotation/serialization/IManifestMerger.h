#pragma once

#include <string>

#include "nlohmann/json.hpp"

namespace dsannotation::serialization {

class IManifestMerger {
public:
    virtual ~IManifestMerger() = default;
    virtual nlohmann::json merge(const std::string& existingPath,
                                 const nlohmann::json& generated) const = 0;
};

} // namespace dsannotation::serialization
