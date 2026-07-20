# DarkTableNext

DarkTableNext is a cross-platform photo-workflow and RAW-editing application
derived from a proven image-processing core. Version 0.9 is a focused
maintenance baseline: it preserves the C/C++ processing pipeline and GTK front
end while removing historical compatibility layers and peripheral features that
no longer belong to the product.

This repository is under active development and is not a general-availability
release. The implementation in `src/` is the source of truth for current
behaviour.

## Supported workflow

DarkTableNext supports local photo import and cataloguing, Lighttable,
Darkroom, non-destructive editing, masks, history, colour management, and
local-disk export. The supported export formats are JPEG, PNG, TIFF, and an
unchanged copy of the original file.

The maintained desktop targets are macOS (Apple Silicon and Intel), Windows,
and Linux. The GTK interface remains part of the application. The CPU pipeline
is always the reference implementation and reliable fallback; OpenCL remains
available only while the planned GPU transition is validated.

The project does not maintain removed Lua support, historical plugins or UI
ABI, scripting, map and tethering workflows, printing, slideshow, remote
publishing, or old format compatibility. For the complete product boundary,
see [TODO_CORE_REDUCTION.md](TODO_CORE_REDUCTION.md).

### Bundled styles

The distribution includes only the generic example styles. The historical
per-camera style catalog is intentionally excluded; users' own styles remain
supported.

## Build from source

Use a recent development environment with CMake 3.26 or newer, Ninja,
`ccache`, and the compiler and libraries required by the generated CMake
presets. macOS uses Homebrew, Windows uses the Visual Studio/vcpkg toolchain,
and Linux uses its distribution packages. Clang is the default toolchain;
matching GCC presets are also available.

Initialise the FreeCM submodule and materialise the pinned dependency sources:

```sh
git submodule update --init FreeCM
python3 configs/source_root_workflow.py --init
python3 configs/source_root_workflow.py --update
```

`--init` is the only dependency step that may access the network. `--update`
uses the active lock to materialise sources locally and regenerate CMake
presets.

Configure, build, and launch the default debug build:

```sh
cmake --preset mac_clang_debug
cmake --build --preset mac_clang_debug
./build/mac_clang_debug/bin/darktable --version
```

Useful macOS presets are `mac_clang_debug`, `mac_clang_release`,
`mac_gcc_debug`, and `mac_gcc_release`; Windows provides matching
`win_msvc_debug` and `win_msvc_release` presets. Linux may be configured with
the generated host preset or a normal out-of-tree CMake build after the pinned
dependencies have been materialised. Configure the macOS release preset and
build the packaging target to produce a local DMG:

```sh
cmake --preset mac_clang_release
cmake --build --preset mac_clang_release --target package-macos
```

The default package is ad-hoc signed for local validation. Distribution
requires an appropriate Developer ID signature and notarisation.

### Tests

Tests are disabled by default. Enable and run the unit-labelled tests with a
debug preset:

```sh
cmake --preset mac_clang_debug -DBUILD_TESTING=ON
cmake --build --preset mac_clang_debug
ctest --test-dir build/mac_clang_debug --output-on-failure -L unit
```

For GPU measurement and comparison procedures, read
[benchmarks/README.md](benchmarks/README.md) and
[DevDocs/GPU_Baseline.md](DevDocs/GPU_Baseline.md).

## Dependency management

`source_roots.lock.jsonc.in` is the version-controlled dependency baseline.
FreeCM generates the local active lock and CMake presets; do not hand-edit or
commit these generated files and directories:

- `source_roots.lock.jsonc`
- `CMakePresets.json`
- `.freecm/`
- `build/dependency_seed_repos/`
- `build/dependency_source_roots/`

Do not add CMake download steps, vendored dependency copies, or replacement
submodules. Permanent dependency changes belong in the lock template,
`configs/source_roots.py`, and the CMake code that consumes them.

## Repository guide

| Path | Purpose |
| --- | --- |
| `src/` | Application code, image pipeline, IOPs, and GTK user interface |
| `cmake/` | CMake modules and build policy |
| `configs/` | FreeCM source-root configuration and workflow scripts |
| `data/` | Runtime resources and configuration data |
| `benchmarks/` | Reproducible performance and GPU-baseline tools |
| `DevDocs/` | Developer documentation and source maps |
| `FreeCM/` | Dependency-management submodule |

Before contributing, read [AGENTS.md](AGENTS.md). In particular, do not change
generated dependency state or FreeCM-managed source directories, and preserve
the CPU image results unless the task explicitly concerns image-processing
behaviour.

## Further reading

- [Release notes](RELEASE_NOTES.md)
- [Core reduction plan](TODO_CORE_REDUCTION.md)
- [C11 consolidation plan](TODO_C11_CONSOLIDATION.md)
- [Developer documentation index](DevDocs/README.md)
- [GPU baseline](DevDocs/GPU_Baseline.md)

## License

DarkTableNext is distributed under the [GNU General Public License, version 3](LICENSE).
