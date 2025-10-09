#pragma once

#include "DSAnnotation/Core/Component.h"

namespace clang {
class ASTContext;
class CXXRecordDecl;
}

namespace dsannotation::parsing {

class IComponentParser {
public:
    virtual ~IComponentParser() = default;
    virtual core::Component parse(const clang::CXXRecordDecl& declaration,
                                  clang::ASTContext& context) const = 0;
};

} // namespace dsannotation::parsing
