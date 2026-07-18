#!/usr/bin/env python3
"""Manage FreeCM source roots and generated CMake presets."""

from __future__ import annotations

import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent
FREECM_ROOT = REPO_ROOT / "FreeCM"

for path in (REPO_ROOT, FREECM_ROOT):
    if str(path) not in sys.path:
        sys.path.insert(0, str(path))

from configs.source_roots import *  # noqa: F401,F403,E402
from repomgrcpp.cmake_workflow import (  # noqa: E402
    CMakeDependencyBuildSpec,
    bind_cmake_workflow_script,
)
from repomgrcpp.preset_templates import resolve_preset_models as resolve_freecm_preset_models  # noqa: E402


def resolve_preset_models(*args: object, **kwargs: object):
    """Keep the generated macOS compiler presets tied to their toolchains."""
    resolved = resolve_freecm_preset_models(*args, **kwargs)
    if resolved.os_group != "mac":
        return resolved

    for model in (resolved.resolved_model, resolved.generated_model):
        for preset in model["configurePresets"]:
            cache = preset["cacheVariables"]
            name = preset["name"]
            if name.startswith("mac_clang_"):
                cache["CMAKE_C_COMPILER"] = "clang"
                cache["CMAKE_CXX_COMPILER"] = "clang++"
            elif name.startswith("mac_gcc_"):
                cache["CMAKE_C_COMPILER"] = "gcc-16"
                cache["CMAKE_CXX_COMPILER"] = "g++-16"
                preset["environment"] = {
                    "PATH": "/opt/homebrew/opt/gcc/bin:/opt/homebrew/opt/llvm/bin:$penv{PATH}"
                }
    return resolved


# The application consumes these source roots with add_subdirectory(), so no
# standalone dependency SDKs need to be built before configuring DarkTableNext.
DEPENDENCY_BUILD_ORDER: tuple[CMakeDependencyBuildSpec, ...] = ()

WORKFLOW_SCRIPT = bind_cmake_workflow_script(
    globals(),
    repo_root=REPO_ROOT,
    repo_display_name="DarkTableNext",
    dependency_build_order=DEPENDENCY_BUILD_ORDER,
)


if __name__ == "__main__":
    raise SystemExit(WORKFLOW_SCRIPT.main())
