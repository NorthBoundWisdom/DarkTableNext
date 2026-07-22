#!/usr/bin/env python3
"""Check that Ravo production sources retain the Phase 1 dependency boundary."""

from __future__ import annotations

import argparse
import re
from pathlib import Path


PRODUCTION_DIRECTORIES = ("foundation", "recipe", "engine", "adapters", "cli")
SOURCE_SUFFIXES = frozenset({".c", ".cc", ".cpp", ".cxx", ".h", ".hpp"})
FORBIDDEN_INCLUDE_PATTERNS = (
    (re.compile(r"^\s*#\s*include\s*[<\"](?:\.\./)*src/", re.MULTILINE), "frozen src header"),
    (re.compile(r"^\s*#\s*include\s*[<\"](?:gtk|dtgtk)/", re.MULTILINE), "GTK header"),
    (re.compile(r"^\s*#\s*include\s*[<\"](?:darktable|libdarktable)", re.MULTILINE), "legacy core header"),
    (re.compile(r"^\s*#\s*include\s*[<\"](?:sqlite|sqlite3)", re.MULTILINE), "catalog database header"),
)
FORBIDDEN_SYMBOL_PATTERNS = (
    (re.compile(r"\b(?:dlopen|LoadLibrary[A-W]?|GetProcAddress)\b"), "dynamic legacy-module loading"),
    (re.compile(r"\b(?:sqlite3?|QSqlDatabase)\b"), "catalog database API"),
)
FORBIDDEN_QT_UI_PATTERN = re.compile(r"\b(?:Qt6::(?:Gui|Widgets|Qml|Quick)|QGui\w*|QWidget\w*|QQml\w*|QQuick\w*)\b")
QT_TOKEN_PATTERN = re.compile(r"\bQt6::([A-Za-z0-9_]+)\b")
TARGET_LINK_PATTERN = re.compile(
    r"target_link_libraries\(\s*([A-Za-z0-9_]+)\s+(.*?)\)", re.DOTALL
)
FIRST_PARTY_TARGET_PATTERN = re.compile(
    r"\b(ravo(?:_foundation|_recipe|_engine|_adapters|_cli)?)\b"
)
PUBLIC_RAVO_INCLUDE_PATTERN = re.compile(r'^\s*#\s*include\s*"ravo/([a-z_]+)/', re.MULTILINE)
ALLOWED_FIRST_PARTY_LINKS = {
    "ravo_foundation": frozenset(),
    "ravo_recipe": frozenset({"ravo_foundation"}),
    "ravo_engine": frozenset({"ravo_foundation", "ravo_recipe"}),
    "ravo_adapters": frozenset({"ravo_foundation", "ravo_recipe"}),
    "ravo_cli": frozenset({"ravo_adapters", "ravo_engine"}),
    "ravo": frozenset({"ravo_cli"}),
}
REQUIRED_FIRST_PARTY_LINKS = {
    "ravo_foundation": frozenset(),
    "ravo_recipe": frozenset({"ravo_foundation"}),
    "ravo_engine": frozenset({"ravo_recipe"}),
    "ravo_adapters": frozenset({"ravo_foundation", "ravo_recipe"}),
    "ravo_cli": frozenset({"ravo_adapters", "ravo_engine"}),
    "ravo": frozenset({"ravo_cli"}),
}
ALLOWED_PUBLIC_HEADER_LAYERS = {
    "foundation": frozenset({"foundation"}),
    "recipe": frozenset({"foundation", "recipe"}),
    "engine": frozenset({"foundation", "recipe", "engine"}),
    "adapters": frozenset({"foundation", "recipe", "adapters"}),
    "cli": frozenset({"foundation", "recipe", "engine", "adapters", "cli"}),
}


class BoundaryError(Exception):
    """Raised when a production source crosses a forbidden Phase 1 boundary."""


def source_files(repository_root: Path) -> list[Path]:
    files: list[Path] = []
    for directory in PRODUCTION_DIRECTORIES:
        root = repository_root / "Ravo" / directory
        if not root.is_dir():
            raise BoundaryError(f"missing Ravo production directory: {root}")
        for path in root.rglob("*"):
            if path.is_file() and (path.suffix in SOURCE_SUFFIXES or path.name == "CMakeLists.txt"):
                files.append(path)
    return sorted(files)


def line_number(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def verify_source(path: Path, repository_root: Path) -> None:
    try:
        text = path.read_text(encoding="utf-8")
    except OSError as error:
        raise BoundaryError(f"cannot read production source {path}: {error}") from error
    relative = path.relative_to(repository_root).as_posix()
    for pattern, description in FORBIDDEN_INCLUDE_PATTERNS:
        match = pattern.search(text)
        if match:
            raise BoundaryError(f"{relative}:{line_number(text, match.start())} includes a {description}")
    for pattern, description in FORBIDDEN_SYMBOL_PATTERNS:
        match = pattern.search(text)
        if match:
            raise BoundaryError(f"{relative}:{line_number(text, match.start())} uses {description}")
    match = FORBIDDEN_QT_UI_PATTERN.search(text)
    if match:
        raise BoundaryError(f"{relative}:{line_number(text, match.start())} uses a desktop Qt API")

def verify_public_header_direction(path: Path, repository_root: Path) -> None:
    relative = path.relative_to(repository_root)
    if len(relative.parts) < 3 or relative.parts[0] != "Ravo" or relative.parts[2] != "include":
        return
    owner = relative.parts[1]
    allowed_layers = ALLOWED_PUBLIC_HEADER_LAYERS.get(owner)
    if allowed_layers is None:
        raise BoundaryError(f"unexpected Ravo production owner for public header: {relative.as_posix()}")
    text = path.read_text(encoding="utf-8")
    for match in PUBLIC_RAVO_INCLUDE_PATTERN.finditer(text):
        included_layer = match.group(1)
        if included_layer not in allowed_layers:
            raise BoundaryError(
                f"{relative.as_posix()}:{line_number(text, match.start())} includes ravo/{included_layer} "
                f"against the {owner} public-header direction"
            )


def verify_qt_cmake_boundary(repository_root: Path) -> None:
    for path in source_files(repository_root):
        if path.name != "CMakeLists.txt":
            continue
        text = path.read_text(encoding="utf-8")
        qt_targets = QT_TOKEN_PATTERN.findall(text)
        if not qt_targets:
            continue
        if any(target != "Core" for target in qt_targets):
            relative = path.relative_to(repository_root).as_posix()
            raise BoundaryError(
                f"{relative} may link Qt6::Core but not a Qt GUI/QML/Widgets target"
            )


def verify_first_party_target_direction(repository_root: Path) -> None:
    links = {target: set() for target in ALLOWED_FIRST_PARTY_LINKS}
    for path in source_files(repository_root):
        if path.name != "CMakeLists.txt":
            continue
        text = path.read_text(encoding="utf-8")
        for match in TARGET_LINK_PATTERN.finditer(text):
            target = match.group(1)
            if target not in links:
                continue
            links[target].update(FIRST_PARTY_TARGET_PATTERN.findall(match.group(2)))
    for target, actual_links in links.items():
        unexpected = actual_links - ALLOWED_FIRST_PARTY_LINKS[target]
        missing = REQUIRED_FIRST_PARTY_LINKS[target] - actual_links
        if unexpected or missing:
            details = []
            if unexpected:
                details.append("unexpected: " + ", ".join(sorted(unexpected)))
            if missing:
                details.append("missing: " + ", ".join(sorted(missing)))
            raise BoundaryError(
                f"first-party dependency direction for {target} is invalid; " + "; ".join(details)
            )


def verify(repository_root: Path) -> None:
    for path in source_files(repository_root):
        verify_source(path, repository_root)
        verify_public_header_direction(path, repository_root)
    verify_qt_cmake_boundary(repository_root)
    verify_first_party_target_direction(repository_root)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repository-root",
        type=Path,
        default=Path(__file__).resolve().parents[2],
        help="DarkTableNext repository root (default: inferred from this script)",
    )
    arguments = parser.parse_args()
    try:
        verify(arguments.repository_root.resolve())
    except BoundaryError as error:
        print(f"Ravo dependency boundary check failed: {error}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
