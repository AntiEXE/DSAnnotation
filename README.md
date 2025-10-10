
# DSAnnotation

DSAnnotation is a modular Clang-based tooling pipeline that scans C++ translation units for Declarative Services annotations and emits a merged OSGi manifest. The refactor introduces composable modules, testable seams, and clearer layering between parsing, serialization, and infrastructure support.

## Project layout

```
include/DSAnnotation/
  Core/           # Domain models (Component, Reference, Error, Result)
  Parsing/        # AST visitor + parser interfaces/implementations
  Serialization/  # Manifest builder, merger, writer abstractions
  Support/        # Error reporting, filesystem, syntax helpers
  Config/         # Runtime configuration objects
src/
  Core/           # Concrete domain types
  Parsing/        # Parsing pipeline implementations
  Serialization/  # JSON generation & manifest merge
  Support/        # Platform services (filesystem, checks)
app/
  main.cpp        # Composition root & Clang tool wiring
tests/
  ...             # GoogleTest unit coverage
```

Legacy headers in the repository root now forward to the new `include/DSAnnotation` hierarchy to ease migration, but new development should include the modular headers directly.

## Build

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The top-level `CMakeLists.txt` exposes:

- `dsannotation_core`, `dsannotation_support`, `dsannotation_parsing`, `dsannotation_serialization` – structured static libraries
- `dsannotation` – CLI executable backed by the modular pipeline
- `dsannotation_tests` – GoogleTest suite (see below)

### Running the tool

```powershell
build\dsannotation.exe --help
build\dsannotation.exe -i path\to\existing\manifest.json -o out\dir source.cpp
```

Output is written to `ParserConfig::outputDirectory / ParserConfig::outputFileName` (default `manifest.json`). Existing manifests are merged so custom bundle metadata is preserved.

### Tests

```powershell
cmake --build build --target dsannotation_tests
ctest --test-dir build
```

Additional unit tests can be added under `tests/` and linked against the modular libraries.

## Design highlights

- **Dependency inversion** – every major subsystem (`IComponentParser`, `IManifestWriter`, `IFileSystem`, etc.) is expressed as an interface. Concrete implementations are composed in `app/main.cpp`, which keeps the rest of the codebase framework-agnostic and easy to mock.
- **Error handling** – `core::ErrorCollector` centralizes diagnostics with formatted source locations, while `support::ErrorReporter` produces human-friendly summaries.
- **Filesystem abstraction** – `support::IFileSystem` decouples file access, simplifying testing (mockable in unit tests) and future portability.
- **Configuration-first** – `config::ParserConfig` captures output paths, validation flags, and formatting preferences, allowing future CLI/UI layers to remain thin.

## Migration notes

- Legacy sources in the repository root are marked as deprecated and forward to the new modular headers. They remain temporarily for backwards compatibility but will be removed after downstream clients migrate.
- Consumers should link against the new CMake targets rather than compiling individual `.cpp` files manually.

## Next steps

- Add focused unit tests for `ComponentParser` using pre-built AST fixtures.
- Provide mocks for `IFileSystem` and `ISyntaxChecker` in the test suite to validate edge cases (e.g., missing property files, brace mismatch reporting).
- Expand integration tests that execute the full tool via `clang::tooling::runToolOnCode`.

