#pragma once

#include <string>

#include "DSAnnotation/Core/Component.h"
#include "DSAnnotation/Core/Result.h"

namespace dsannotation::serialization {

class IManifestWriter {
public:
    virtual ~IManifestWriter() = default;

    virtual core::Result<bool> writeManifest(const core::ComponentList& components,
                                             const std::string& existingManifestPath,
                                             const std::string& outputPath) const = 0;
};

} // namespace dsannotation::serialization
