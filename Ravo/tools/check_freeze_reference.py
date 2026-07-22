#!/usr/bin/env python3
"""Verify that Ravo development has not changed the frozen 0.9 source or fixtures."""

from __future__ import annotations

import argparse
import subprocess
from pathlib import Path


DEFAULT_FREEZE_COMMIT = "320970bf7c9cbbc6611cfc3eb60f8f2b0424b782"
# Ravo owns its independent CMake graph under Ravo/. These paths instead make
# up the frozen 0.9 application source, build graph, bundled resources, package
# graph, and regression corpus named by ADR-0004.
PROTECTED_PATHS = ("src", "darktable-tests", "CMakeLists.txt", "cmake", "data", "packaging")


class FreezeCheckError(Exception):
    """Raised when a protected path differs from the recorded freeze reference."""


def run_git(repository_root: Path, *arguments: str) -> str:
    completed = subprocess.run(
        ("git", *arguments),
        cwd=repository_root,
        check=False,
        capture_output=True,
        text=True,
    )
    if completed.returncode != 0:
        detail = completed.stderr.strip() or completed.stdout.strip()
        raise FreezeCheckError(f"git {' '.join(arguments)} failed: {detail}")
    return completed.stdout.strip()


def verify(repository_root: Path, freeze_commit: str) -> dict[str, str]:
    freeze = run_git(repository_root, "rev-parse", f"{freeze_commit}^{{commit}}")
    head = run_git(repository_root, "rev-parse", "HEAD^{commit}")
    run_git(repository_root, "merge-base", "--is-ancestor", freeze, head)

    trees: dict[str, str] = {}
    for path in PROTECTED_PATHS:
        frozen_tree = run_git(repository_root, "rev-parse", f"{freeze}:{path}")
        current_tree = run_git(repository_root, "rev-parse", f"HEAD:{path}")
        if frozen_tree != current_tree:
            raise FreezeCheckError(
                f"committed {path} object differs from the frozen reference: "
                f"{frozen_tree} != {current_tree}"
            )
        trees[path] = frozen_tree
    worktree_changes = run_git(
        repository_root,
        "status",
        "--porcelain",
        "--untracked-files=all",
        "--",
        *PROTECTED_PATHS,
    )
    if worktree_changes:
        raise FreezeCheckError(
            "working-tree changes exist under frozen paths: "
            + ", ".join(line[3:] for line in worktree_changes.splitlines())
        )
    return trees


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repository-root",
        type=Path,
        default=Path(__file__).resolve().parents[2],
        help="DarkTableNext repository root (default: inferred from this script)",
    )
    parser.add_argument(
        "--freeze-commit",
        default=DEFAULT_FREEZE_COMMIT,
        help="immutable 0.9 freeze commit (default: %(default)s)",
    )
    arguments = parser.parse_args()
    try:
        trees = verify(arguments.repository_root.resolve(), arguments.freeze_commit)
    except FreezeCheckError as error:
        print(f"freeze reference check failed: {error}")
        return 1
    print(
        f"freeze reference verified: commit={arguments.freeze_commit} "
        f"src={trees['src']} darktable-tests={trees['darktable-tests']} "
        f"protected_paths={len(PROTECTED_PATHS)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
