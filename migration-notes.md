# Migration script (hypothetical - don't run this)
# Move headers from include/DSAnnotation/* to include/*
mv include/DSAnnotation/Core include/
mv include/DSAnnotation/Parsing include/
mv include/DSAnnotation/Serialization include/
mv include/DSAnnotation/Support include/
mv include/DSAnnotation/Config include/
rmdir include/DSAnnotation

# Update all #include statements from:
#   #include "DSAnnotation/Core/Component.h"
# to:
#   #include "Core/Component.h"

# Update CMakeLists.txt include directories