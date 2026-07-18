#!/usr/bin/env python3
"""Manage the optional benchmark image source root with FreeCM."""

from __future__ import annotations

import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent
BENCHMARK_ROOT = REPO_ROOT / "src" / "tests" / "benchmark"
FREECM_ROOT = REPO_ROOT / "FreeCM"
for path in (REPO_ROOT, FREECM_ROOT):
    if str(path) not in sys.path:
        sys.path.insert(0, str(path))

from freecm.dependency_roots import (  # noqa: E402
    DependencyRootConfig,
    DependencyRootSpec,
    bind_dependency_root_workflow,
)


workflow = bind_dependency_root_workflow(
    globals(),
    DependencyRootConfig(
        repo_root=BENCHMARK_ROOT,
        dependency_root_specs=(
            DependencyRootSpec(
                dependency_name="darktable-tests",
                repo_name="darktable-tests",
                env_key="DARKTABLE_TESTS_SOURCE_ROOT",
                required_relative_paths=("images/mire1.cr2",),
            ),
        ),
        repo_display_name="DarkTableNext benchmark assets",
    ),
)


if __name__ == "__main__":
    raise SystemExit(main())
