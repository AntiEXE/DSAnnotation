#pragma once

#include "DSAnnotation/Core/Component.h"
#include "nlohmann/json.hpp"

namespace dsannotation::serialization {

class IManifestBuilder {
public:
    virtual ~IManifestBuilder() = default;
    virtual nlohmann::json buildManifest(const core::ComponentList& components) const = 0;
};

} // namespace dsannotation::serialization
