# DarkTableNext 0.9.0 Release Notes

DarkTableNext 0.9.0 establishes the project's independent maintenance baseline.
It keeps the proven photo-processing core and GTK interface while narrowing the
product to a focused desktop workflow.

## Highlights

- macOS (Apple Silicon and Intel), Windows, and Linux are maintained build
  targets.
- CMake is the supported build entry point.
- FreeCM manages pinned third-party source roots through
  `source_roots.lock.jsonc.in`.
- The application retains local import and cataloguing, Lighttable, Darkroom,
  non-destructive editing, colour management, history, masks, and local-disk
  export.
- The existing CPU image pipeline remains the correctness reference. OpenCL is
  still available during the planned GPU transition.

## Scope reduction

This baseline intentionally removes historical functionality that is outside
the current product boundary, including Lua support, slideshow, printing, map,
tethering, MIDI and game-controller input, email and Piwigo export, and AI or
ONNX features. Local export is limited to JPEG, PNG, TIFF, and copying the
original file.

Support for removed plugins, scripting interfaces, formats, packaging paths,
and legacy UI behaviour is not provided.

## Development status

Version 0.9.0 is a maintenance and reduction baseline, not a general-availability
release. The immediate work is to finish the remaining legacy-code audit and
freeze the long-term IOP boundary. Metal replaces OpenCL only after the
documented correctness and performance gates are met; until then, OpenCL must
remain intact.

See [the README](README.md) for build instructions and
[the core reduction plan](TODO_CORE_REDUCTION.md) for the maintained product
boundary and roadmap.
