---
name: build-repo
description: Configure, build, test, run, or diagnose DarkTableNext using its FreeCM-managed source roots and CMake presets. Use for dependency materialization, build failures, validation after code changes, test selection, or choosing a Debug/Release and Clang/GCC preset.
---

# Build DarkTableNext

Use the active FreeCM workspace and generated CMake presets. Keep dependency preparation,
configuration, compilation, testing, and runtime diagnosis as separate observable steps.

## Establish the workspace

1. Read the root `AGENTS.md` and `README.md`.
2. Run `git status --short --branch` and preserve existing changes.
3. Check that `FreeCM/`, `source_roots.lock.jsonc`, and `CMakePresets.json` exist.
4. If the workspace has never been prepared, run:

   ```sh
   git submodule update --init FreeCM
   python3 configs/source_root_workflow.py --init
   python3 configs/source_root_workflow.py --update
   ```

`--init` is the only network-enabled dependency step. Use `--update` to materialize the
active lock offline and regenerate presets.

Do not edit or build inside `build/dependency_seed_repos` or
`build/dependency_source_roots`. They are parent-managed artifacts. Durable dependency
changes belong in `source_roots.lock.jsonc.in`, `configs/source_roots.py`, and consuming
CMake files. The only intended hand-edit of the ignored active lock is a local `manual`
override for linked dependency development.

## Select a build

Use `mac_clang_debug` for normal development and diagnostics:

```sh
cmake --preset mac_clang_debug
cmake --build --preset mac_clang_debug
```

Use `mac_clang_release` for performance, packaging-like staging, or representative GPU
measurements. Use the GCC presets only for compiler coverage or a GCC-specific issue.
Use `mac_xcode` only when an Xcode project is materially useful.

Build a named target when that is sufficient:

```sh
cmake --build --preset mac_clang_debug --target darktable
```

Run the built application or version check directly from the selected preset directory:

```sh
./build/mac_clang_debug/bin/darktable --version
```

## Enable and run tests

Tests are opt-in. Reconfigure explicitly before claiming test coverage:

```sh
cmake --preset mac_clang_debug -DBUILD_TESTING=ON
cmake --build --preset mac_clang_debug
ctest --test-dir build/mac_clang_debug --output-on-failure -L unit
```

For a focused failure, use `ctest --test-dir build/mac_clang_debug -N` to discover names,
then select with `-R`. Do not report tests as passing if the preset was last configured with
`BUILD_TESTING=OFF`.

## Diagnose failures

Classify the earliest real failure before editing:

- dependency root missing or wrong commit: inspect the active lock, then rerun `--update`;
- generated preset stale: change the template/generator if needed, then rerun `--update`;
- configure failure: capture the first missing package, compiler, or CMake diagnostic;
- compile failure: rebuild the narrow target with verbose output when needed;
- link/runtime failure: inspect the selected binary and loader paths, not a different preset;
- stale plugin or data artifact: install into a new staging prefix before drawing conclusions.

Do not install Homebrew packages or change system configuration unless the user authorized it.
Do not hide a failure by turning off a required feature.

## Validation depth

- Documentation-only changes: check Markdown links, commands, and `git diff --check`.
- CMake or dependency changes: run `--update`, configure, and build an affected target.
- C/C++ changes: build the affected target and run relevant tests.
- Broad core changes: run the complete `unit` label.
- GPU or performance changes: follow `DevDocs/GPU_Baseline.md` with a Release build.

Report the preset, targets, tests, result, and any checks not run. A missing dependency or
asset is a limitation, not a passing result.
