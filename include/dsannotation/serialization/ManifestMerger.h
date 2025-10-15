#pragma once

#include <string>

#include "dsannotation/serialization/IManifestMerger.h"
#include "dsannotation/support/IFileSystem.h"

namespace dsannotation::serialization {

class ManifestMerger final : public IManifestMerger {
public:
    explicit ManifestMerger(const support::IFileSystem& fileSystem);

    nlohmann::json merge(const std::string& existingPath,
                         const nlohmann::json& generated) const override;

private:
    nlohmann::json readExistingManifest(const std::string& path) const;
    void mergeComponents(nlohmann::json& target, const nlohmann::json& source) const;

    const support::IFileSystem& fileSystem_;
};

} // namespace dsannotation::serialization
