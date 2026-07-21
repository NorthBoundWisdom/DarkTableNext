#!/usr/bin/env python3
"""Build a deterministic inventory of the legacy image-regression assets.

The manifest deliberately records committed inputs and reference files without
claiming that the current legacy executable reproduced them.  Runtime evidence
belongs in the accompanying phase-0 baseline report.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path
from typing import Any


OPERATION_PATTERN = re.compile(r'operation="([^"]+)"')
# The document version is an attribute on the rdf:Description element, not a
# <darktable> XML element. Record that actual legacy XMP version so a fixture
# hash cannot conceal a schema change.
SCHEMA_PATTERN = re.compile(r'darktable:xmp_version="([^"]+)"')


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def relative(root: Path, path: Path) -> str:
    return path.relative_to(root).as_posix()


def file_record(root: Path, path: Path) -> dict[str, Any]:
    return {
        "path": relative(root, path),
        "sha256": sha256(path),
        "size_bytes": path.stat().st_size,
    }


def xmp_metadata(path: Path) -> tuple[list[str], list[str]]:
    text = path.read_text(encoding="utf-8", errors="replace")
    return (
        sorted(set(OPERATION_PATTERN.findall(text))),
        sorted(set(SCHEMA_PATTERN.findall(text))),
    )


def build_manifest(repository_root: Path) -> dict[str, Any]:
    tests_root = repository_root / "darktable-tests"
    fixture_directories = sorted(
        path for path in tests_root.iterdir() if path.is_dir() and re.match(r"^\d{4}-", path.name)
    )

    fixtures: list[dict[str, Any]] = []
    all_operations: set[str] = set()
    all_schema_versions: set[str] = set()
    for directory in fixture_directories:
        xmp_files = sorted(directory.glob("*.xmp"))
        xmp_metadata_records: list[dict[str, Any]] = []
        for xmp_path in xmp_files:
            operations, schema_versions = xmp_metadata(xmp_path)
            all_operations.update(operations)
            all_schema_versions.update(schema_versions)
            xmp_metadata_records.append(
                {
                    **file_record(repository_root, xmp_path),
                    "operations": operations,
                    "schema_versions": schema_versions,
                }
            )

        references = [
            file_record(repository_root, path)
            for path in sorted(directory.glob("expected.png"))
        ]
        cpu_gpu_limits = [
            {
                **file_record(repository_root, path),
                "maximum_differing_pixels": path.read_text(encoding="ascii").strip(),
            }
            for path in sorted(directory.glob("cpugpu.maxpix"))
        ]
        fixtures.append(
            {
                "id": directory.name,
                "xmp": xmp_metadata_records,
                "expected_png": references,
                "cpu_gpu_limits": cpu_gpu_limits,
                "has_custom_driver": (directory / "test.sh").is_file(),
            }
        )

    image_records = [
        file_record(repository_root, path)
        for path in sorted((tests_root / "images").iterdir())
        if path.is_file()
    ]
    return {
        "manifest_schema_version": 1,
        "state": "inventory-only",
        "notice": (
            "Hashes identify committed legacy assets. They do not certify that a legacy "
            "CPU run reproduced expected_png; consult Ravo/docs/phase0/legacy-baseline-2026-07-21.md."
        ),
        "fixture_count": len(fixtures),
        "legacy_xmp_schema_versions": sorted(all_schema_versions),
        "operations": sorted(all_operations),
        "source_images": image_records,
        "fixtures": fixtures,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repository-root",
        type=Path,
        default=Path(__file__).resolve().parents[2],
        help="DarkTableNext repository root (default: inferred from this script)",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=None,
        help="manifest path (default: Ravo/tests/fixtures/legacy_manifest.json)",
    )
    arguments = parser.parse_args()
    repository_root = arguments.repository_root.resolve()
    output = arguments.output or repository_root / "Ravo" / "tests" / "fixtures" / "legacy_manifest.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        json.dumps(build_manifest(repository_root), indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
