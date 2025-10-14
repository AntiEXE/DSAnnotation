#pragma once

#include "DSAnnotation/Serialization/IManifestBuilder.h"
#include "DSAnnotation/Serialization/IManifestMerger.h"
#include "DSAnnotation/Serialization/IManifestWriter.h"
#include "DSAnnotation/Support/IFileSystem.h"

namespace dsannotation::serialization {

class JsonManifestWriter final : public IManifestWriter {
public:
    JsonManifestWriter(const IManifestBuilder& builder,
                       const IManifestMerger& merger,
                       const support::IFileSystem& fileSystem,
                       int indentation = 4);

    core::Result<bool> writeManifest(const core::ComponentList& components,
                                     const std::string& existingManifestPath,
                                     const std::string& outputPath) const override;

private:
    const IManifestBuilder& builder_;
    const IManifestMerger& merger_;
    const support::IFileSystem& fileSystem_;
    int indentation_;
};

} // namespace dsannotation::serialization
