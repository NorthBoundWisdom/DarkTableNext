#!/usr/bin/env python3
"""Initialize and update the FreeCM-managed dependency source roots."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent
FREECM_ROOT = REPO_ROOT / "FreeCM"

for path in (REPO_ROOT, FREECM_ROOT):
    if str(path) not in sys.path:
        sys.path.insert(0, str(path))

from configs.source_roots import workflow


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Manage DarkTableNext FreeCM source-root state."
    )
    action = parser.add_mutually_exclusive_group(required=True)
    action.add_argument(
        "--init",
        action="store_true",
        help="Create the active lock file and clone or refresh dependency seed repositories.",
    )
    action.add_argument(
        "--update",
        action="store_true",
        help="Materialize the pinned source roots using only local seed repositories.",
    )
    parser.add_argument("--quiet", action="store_true", help="Suppress git command output.")
    return parser


def _print_progress(action: str, message: str, _: str) -> None:
    print(f"[freecm] {action}: {message}")


def main() -> int:
    args = _parser().parse_args()
    if args.init:
        lock_path, created = workflow.ensure_active_lock_file(REPO_ROOT)
        print(
            f"[freecm] init: {'created' if created else 'using'} active lock: {lock_path}"
        )
        closure = workflow.prepare_seed_repository_closure(
            REPO_ROOT,
            progress=_print_progress,
            quiet=args.quiet,
        )
        print(f"[freecm] init: prepared {len(closure.topo_order)} dependency seed repositories")
        return 0

    source_roots = workflow.materialize_dependency_roots(
        REPO_ROOT,
        allow_network=False,
        quiet=args.quiet,
    )
    problems = workflow.validate_dependency_roots(source_roots)
    if problems:
        raise FileNotFoundError("Source roots are not ready:\n- " + "\n- ".join(problems))
    print(f"[freecm] update: materialized {len(source_roots.closure_order)} dependency source roots")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
