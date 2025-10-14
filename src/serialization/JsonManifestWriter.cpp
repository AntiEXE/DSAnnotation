#include "dsannotation/serialization/JsonManifestWriter.h"

#include <exception>

namespace dsannotation::serialization {

JsonManifestWriter::JsonManifestWriter(const IManifestBuilder& builder,
                                       const IManifestMerger& merger,
                                       const support::IFileSystem& fileSystem,
                                       int indentation)
    : builder_(builder),
      merger_(merger),
      fileSystem_(fileSystem),
      indentation_(indentation) {}

core::Result<bool> JsonManifestWriter::writeManifest(const core::ComponentList& components,
                                                     const std::string& existingManifestPath,
                                                     const std::string& outputPath) const {
    try {
        auto generated = builder_.buildManifest(components);
        auto merged = merger_.merge(existingManifestPath, generated);

        const bool success = fileSystem_.writeTextFile(outputPath,
                                                       merged.dump(indentation_));
        if (!success) {
            return core::Result<bool>::error("Failed to write manifest to " + outputPath);
        }

        return core::Result<bool>::success(true);
    } catch (const std::exception& ex) {
        return core::Result<bool>::error(ex.what());
    }
}

} // namespace dsannotation::serialization
