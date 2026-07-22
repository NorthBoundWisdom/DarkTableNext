# ADR-0005: Allow Qt6 Core throughout the headless project

- Status: Accepted
- Date: 2026-07-22

## Context

Ravo's CLI, recipe model, and engine need dependable Unicode paths, files,
JSON, time, and general runtime primitives. Restricting QtCore to one adapter
would create forwarding interfaces even when a direct QtCore implementation is
clearer and faster to migrate.

The sibling RobimPCR repository already uses Qt 6.11.1's `QString`, `QFile`,
`QFileInfo`, and `QSaveFile` for this class of adapter work. The local Windows
toolchain has the same Qt SDK installed at configuration time.

## Decision

- Ravo requires Qt 6.11 or newer with the `Core` component only.
- Any Ravo target may link and use `Qt6::Core` when it simplifies the
  implementation. Do not create an adapter whose only purpose is hiding a
  straightforward QtCore value or utility.
- Stable recipes and machine JSON remain versioned data contracts; they do not
  serialize Qt object memory layouts.
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
- Future atomic recipe/export writes can use `QSaveFile` directly from the
  owning Ravo implementation instead of reimplementing platform replacement.

## Rejected alternatives

- **Path handling in every CLI command**: it repeats platform encoding and
  leaves engine clients with inconsistent failure semantics.
- **Expose `std::filesystem::path` from the CLI/engine contract**: its native
  encoding and lifetime semantics are not the machine-facing Ravo contract.
- **Qt GUI/QML adoption now**: it violates the headless-first delivery order
  and would prematurely choose the future desktop framework.
