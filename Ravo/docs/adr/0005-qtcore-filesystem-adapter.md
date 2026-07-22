# ADR-0005: Use Qt6 Core for private filesystem adapters

- Status: Accepted
- Date: 2026-07-22

## Context

Ravo's CLI, recipe model, and engine need one deterministic boundary for
UTF-8 input paths, native filesystem encodings, bounded reads, error mapping,
and later atomic output. Letting each CLI command translate paths through
`std::filesystem` would duplicate Windows wide-character handling and make
the engine's portable contract depend on platform path types.

The sibling RobimPCR repository already uses Qt 6.11.1's `QString`, `QFile`,
`QFileInfo`, and `QSaveFile` for this class of adapter work. The local Windows
toolchain has the same Qt SDK installed at configuration time.

## Decision

- Ravo requires Qt 6.11 or newer with the `Core` component only.
- `Qt6::Core` is linked privately by `ravo_adapters`; no Qt header or type may
  appear in foundation, recipe, engine, CLI, or a public Ravo adapter header.
- The adapter accepts an unretained UTF-8 path string, creates a private
  `QString`, and maps `QFile`/`QFileInfo` failures to Ravo `TaskError` values.
- FreeCM's generated root CMake preset supplies the Qt SDK location. The Ravo
  preset inherits it rather than hard-coding a path.
- Windows CMake copies the `Qt6::Core` runtime beside `ravo` and the contract
  test executable. The install graph installs the imported Qt Core runtime.
- Qt GUI, QML, Widgets, Quick, and desktop targets remain out of scope through
  the headless exit. This decision does not select Ravo Studio's UI framework.
- Ravo is GPLv3; distributing the Qt Core runtime must retain the applicable
  Qt license notices and runtime obligations in the eventual packaging work.

## Consequences

- CLI and engine callers use stable UTF-8 strings rather than platform path
  objects, while Windows and future platform encoding rules stay in adapters.
- The build needs the FreeCM-managed Qt CMake prefix in addition to the
  existing vcpkg test toolchain. CMake does not download Qt, and Ravo adds no
  production dependency on the frozen `src` graph.
- Future atomic recipe/export writes use `QSaveFile` in the same adapter layer
  instead of reimplementing platform replacement behavior per command.

## Rejected alternatives

- **Path handling in every CLI command**: it repeats platform encoding and
  leaves engine clients with inconsistent failure semantics.
- **Expose `std::filesystem::path` from the CLI/engine contract**: its native
  encoding and lifetime semantics are not the machine-facing Ravo contract.
- **Qt GUI/QML adoption now**: it violates the headless-first delivery order
  and would prematurely choose the future desktop framework.
