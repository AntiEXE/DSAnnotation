#pragma once

#include <string>

namespace dsannotation::support {

struct SourceLocationInfo {
    std::string filename;
    unsigned int line{0};
    unsigned int column{0};

    SourceLocationInfo() = default;
    
    SourceLocationInfo(std::string file, unsigned int ln, unsigned int col)
        : filename(std::move(file)), line(ln), column(col) {}

    bool isValid() const {
        return !filename.empty() && line > 0;
    }
};

} // namespace dsannotation::support
