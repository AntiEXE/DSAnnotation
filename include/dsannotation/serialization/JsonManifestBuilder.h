#pragma once

#include "dsannotation/serialization/IManifestBuilder.h"

namespace dsannotation::serialization {

class JsonManifestBuilder final : public IManifestBuilder {
public:
    nlohmann::json buildManifest(const core::ComponentList& components) const override;
};

} // namespace dsannotation::serialization
