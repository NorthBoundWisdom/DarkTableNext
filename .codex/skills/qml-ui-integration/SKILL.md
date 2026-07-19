---
name: qml-ui-integration
description: Design or implement DarkTableNext Qt/QML UI work that uses GeoControls, including dependency hookup, linked two-repository development, Qt adapters, domain controls, and vertical slices. Use whenever a task adds QML, changes GeoControls for DarkTableNext, or decides which repository owns a UI capability.
---

# QML UI Integration

Build the replacement UI as a set of tested vertical slices. Preserve the current image
core while establishing explicit Qt ownership, lifetime, threading, and history boundaries.

## Read the contract

Before editing, read:

1. root `AGENTS.md`;
2. `GEOCONTROLS_INTEGRATION.md`;
3. the current core source and relevant `DevDocs/` pages;
4. GeoControls `AGENTS.md`, CMake, and public QML modules when its checkout is available.

The current host UI is GTK3. Qt/QML is a target architecture until the main build actually
contains it; do not delete the working GTK path merely because a QML mockup exists.

## Decide ownership first

Put a capability in GeoControls only if it is complete without DarkTableNext types or photo
semantics. Typical GeoControls work includes:

- theme and control-state tokens;
- generic input, selection, popup, dialog, expander, list, tree, and grid mechanics;
- generic numeric mapping, ranges, formatting, preview/commit signal timing, and gradients;
- reusable curve/plot/scene-graph primitives driven by generic data;
- accessibility, focus, keyboard, pointer behavior, demos, lint, and component tests.

Keep these in DarkTableNext:

- IOP and `ParameterDescriptor` models;
- history transactions, undo/redo, presets, module instances, masks, and trouble state;
- color-space and color-management semantics;
- histogram/waveform/vectorscope data and behavior;
- ImageCanvas, Lighttable, filmstrip, crop/perspective/mask/retouch overlays;
- catalog, pixelpipe, import/export, navigation, product actions, icons, and copy.

Use three modules conceptually: `GeoControls`, `DarkTableNext.Controls`, and
`DarkTableNext.App`. Never introduce a reverse dependency from GeoControls to the host.

## Connect the dependency

The current lock template and `configs/source_roots.py` may already register a GeoControls
candidate source root. Materialization alone is not product integration. If the main CMake
graph or the Qt application target is missing, treat first integration as a tracked
architecture change:

1. audit or add its manual-path field, remote, and reachable pinned commit in
   `source_roots.lock.jsonc.in`;
2. add a `DependencyRootSpec` in `configs/source_roots.py`;
3. add Qt 6.8+ and the GeoControls source root to the CMake graph;
4. disable `GEOCONTROLS_BUILD_DEMO` when GeoControls is a subproject;
5. add the smallest host QML module, adapter, executable path, and tests;
6. run `--init`, `--update`, configure, and build.

Do not use `FetchContent`, an unversioned include path, or copied GeoControls sources.

For linked work, edit the ignored active `source_roots.lock.jsonc` to use
`depsMode: "manual"` and point `depsManualPath.GeoControls` to a real independent checkout.
Never point it at `build/dependency_seed_repos`, `build/dependency_source_roots`, or an install
directory. Run `python3 configs/source_root_workflow.py --update` after switching.

## Implement a vertical slice

Prefer one real path such as a single IOP parameter panel with live preview, committed
history, and image output. For each slice:

1. define the core snapshot/model and who owns it;
2. expose only `QObject`, `QAbstractItemModel`, or copied value data to QML;
3. separate high-frequency preview from the single committed history transaction;
4. queue worker/pixelpipe work and cancellation; keep database and processing off the QML
   thread;
5. compose domain controls from GeoControls primitives;
6. test reset, undo/redo, image switch, object destruction, failure, and cancellation;
7. compare behavior with the existing core semantics, not the GTK widget hierarchy.

Do not let QML store a naked C pointer, call an IOP callback directly, or infer validity from
an object that may be destroyed on image switch.

## Validate both repositories

GeoControls changes require its standalone configure/build, demo or focused component test,
and QML lint. Host changes require the selected DarkTableNext configure/build, relevant CTest,
and reproducible manual steps for the slice. Image-processing changes must also preserve CPU
results and follow `DevDocs/GPU_Baseline.md` when a GPU route is involved.

Before formal integration, verify that GeoControls has an explicit GPLv3-compatible license,
third-party resource provenance, and a minimum automated test/lint entry point.

## Finish a cross-repository change

1. Review, commit, and push GeoControls first.
2. Confirm the dependency commit is remotely reachable.
3. Pin that commit in DarkTableNext's tracked lock template.
4. Return to pinned mode, run `--init` if the new commit must be fetched, then `--update`.
5. Reconfigure and validate the host.
6. Commit the host adapter, QML, lock template, and documentation separately.

Never leave the host pinned to an unpushed commit or describe a manual dirty checkout as a
reproducible build.
