# Legacy Baseline Report: 2026-07-21

## Scope and host

This report was produced before Ravo source targets existed.  It records the
legacy 0.9 evidence that can be used later as an independent oracle; it does
not make Ravo pass or fail any image comparison.

| Field | Value |
| --- | --- |
| Host | Windows x64 |
| CMake | 4.4.0 |
| Generator/preset | Ninja / `win_msvc_release` |
| Compiler | MSVC 19.51.36248.0, x64 |
| Legacy image backend | CPU requested with `--disable-opencl` |
| Image fixtures | 158 XMP files, 158 `expected.png` files, 5 source images |
| Legacy XMP schema | 6 |
| Covered operation names | 68 |

The fixture hashes and exact paths are in
[`../../tests/fixtures/legacy_manifest.json`](../../tests/fixtures/legacy_manifest.json).
The manifest state is intentionally `inventory-only` until a successful CPU
run validates the stored PNGs on this toolchain.

## Legacy CTest result

The historical CTest targets were initially absent because the existing build
used `BUILD_TESTING=OFF` and did not have CMocka. After the workspace CMocka
installation, the following commands completed without changing expected-image
files. The build also contained investigative Windows source changes described
below, so this result is diagnostic evidence rather than certification of the
frozen 0.9 source:

```powershell
cmake --preset win_msvc_release -DBUILD_TESTING=ON
cmake --build --preset win_msvc_release --target darktable-test-variables test_sample test_database_schema
ctest --test-dir build/win_msvc_release --output-on-failure -L unit
```

Result: 3/3 passed in 0.15 seconds (`darktable-test-variables`, `test_sample`,
and `test_database_schema`).  The passing empty sample remains a legacy test
asset only; Ravo will not copy it as evidence of engine behavior.

## Image-regression result

The runner was invoked in CPU-only, no-write mode:

```bash
DARKTABLE_CLI=/d/Ravo/build/win_msvc_release/bin/darktable-cli.exe ./run \
  --disable-opencl --disable-timing --no-deltae --fast-fail
```

The initial unchanged Windows run stopped at `0000-nop` before rendering
because no IOP module loaded. An investigative working-tree build then gave
first-party `MODULE` targets the `lib` prefix expected by GLib, exported module
API functions, and moved runtime DLLs out of plugin scan directories.
`dumpbin /exports libagx.dll` reported `dt_module_dt_version`,
`dt_module_mod_version`, and IOP API exports such as `name`; that build loaded
the IOP registry without reporting third-party DLLs as failed modules.

The investigative build also used the pinned FreeCM Exiv2 source root with
`EXIV2_ENABLE_XMP=ON` because the installed vcpkg Exiv2 lacked XMP support. Its
configure/build and all three legacy unit tests succeeded.

The direct `0000-nop` invocation now reaches legacy processing and stops before
PNG output because the LensFun database directory is absent at
`build/win_msvc_release/share/lensfun/version_1`. Existing history diagnostics
also report unsupported blend parameters. The local vcpkg LensFun package
contains the DLL, headers, and metadata but no XML database. NumPy is absent,
so `darktable-tests/count-diff-pixels` returns its fallback value of `50000`.
Neither condition is an image result or a valid reason to change an expected
PNG. Under [ADR-0004](../adr/0004-freeze-09-ravo-only-growth.md) these source
experiments are not a new 0.9 implementation plan and must not be extended to
repair LensFun, history, module loading, or packaging. Ravo records the
unchanged oracle limitation and proceeds from frozen fixtures or an unchanged
reproducible legacy binary.

No expected PNG, XMP, threshold, or source image was written.  The resulting
classification is:

| Run set | Result | Reason |
| --- | --- | --- |
| Legacy CTest unit label | Diagnostic pass, 3/3 | Investigative Windows build; not frozen 0.9 certification |
| `darktable-tests` CPU fixture 0000 | Frozen-oracle result unavailable | Unchanged build failed module loading; investigative build then lacked LensFun XML and emitted blend diagnostics |
| Remaining 157 image fixtures | Not run | Fast-fail stopped after fixture 0000 |
| OpenCL comparison | Not run | CPU-only baseline command and unavailable runner |

## Required follow-up for the Ravo baseline

1. Record the exact frozen 0.9 commit and, if available, an unchanged legacy
   binary/toolchain that can reproduce fixtures on a supported host. Do not
   modify `src` to make this Windows run pass.
2. Keep `legacy_manifest.json` at `inventory-only` where runtime reproduction
   is unavailable. Checked-in RAW/XMP/PNG hashes remain useful evidence, but
   they are not a newly validated CPU run.
3. Run all 158 fixtures CPU-only only with the recorded unchanged binary and
   without `--update-expected`; save command, configuration, logs, and failures.
4. Classify every fixture as reproduced legacy baseline, frozen-oracle failure,
   missing asset, product-approved incompatibility, or deferred Ravo operation.
5. Add Ravo-owned canonical recipe, float TIFF, metadata, and performance
   evidence for the first vertical slice. Never fix the legacy LensFun/blend
   path as a prerequisite for Ravo implementation.
