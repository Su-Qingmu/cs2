#!/usr/bin/env python3
"""Generate a Chinese build of the overlay menu from the shared source file.

The overlay ships one English and one Chinese executable. Rather than keeping a
second, hand-maintained copy of ``menu.hpp`` (which silently drifts the moment
upstream touches the file), the Chinese build is produced by rewriting the
narrow string literals of ``menu.hpp`` using the translation table in
``i18n/zh_map.json``.

Guarantees:

* only *narrow* literals are rewritten -- wide literals (``L"..."``) are left
  alone, which keeps persistence keys and other storage identifiers intact;
* literals that act as ImGui IDs (starting with ``##``) are never rewritten;
* the set of ``printf`` conversion specifiers must match between the English
  source literal and its translation, otherwise the literal is left untouched
  and reported;
* comments and preprocessor directives are never rewritten.

Usage::

    python3 scripts/translate_menu.py --map i18n/zh_map.json \\
        --source external-cheat-base/src/features/menu.hpp \\
        --output external-cheat-base/src/features/menu_zh.hpp

    python3 scripts/translate_menu.py --map i18n/zh_map.json \\
        --source external-cheat-base/src/features/menu.hpp --in-place
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

# printf-style conversion specifier, including the "%%" escape.
FORMAT_SPEC = re.compile(
    r"%(?:%|[-+ #0]*[0-9]*(?:\.[0-9]+)?(?:hh|h|ll|l|z|j|t|L)?[a-zA-Z])"
)

IDENTIFIER_PREFIX = re.compile(r"[A-Za-z0-9_]$")


class Literal:
    """A single string literal found in the source."""

    __slots__ = ("start", "end", "body", "wide", "raw")

    def __init__(self, start: int, end: int, body: str, wide: bool, raw: str) -> None:
        self.start = start
        self.end = end
        self.body = body
        self.wide = wide
        self.raw = raw


def find_literals(source: str) -> list[Literal]:
    """Return every string literal in *source*, ignoring comments."""
    literals: list[Literal] = []
    index = 0
    length = len(source)
    in_block_comment = False
    in_line_comment = False

    while index < length:
        char = source[index]

        if in_line_comment:
            if char == "\n":
                in_line_comment = False
            index += 1
            continue

        if in_block_comment:
            if source.startswith("*/", index):
                in_block_comment = False
                index += 2
                continue
            index += 1
            continue

        if source.startswith("//", index):
            in_line_comment = True
            index += 2
            continue

        if source.startswith("/*", index):
            in_block_comment = True
            index += 2
            continue

        if char == "'":
            # Character literal: skip to the closing quote.
            index += 1
            while index < length:
                if source[index] == "\\":
                    index += 2
                    continue
                if source[index] == "'":
                    index += 1
                    break
                index += 1
            continue

        if char == '"':
            wide = bool(IDENTIFIER_PREFIX.search(source[:index]))
            start = index
            index += 1
            body_start = index
            while index < length:
                if source[index] == "\\":
                    index += 2
                    continue
                if source[index] == '"':
                    break
                index += 1
            body = source[body_start:index]
            index += 1  # consume the closing quote
            literals.append(Literal(start, index, body, wide, source[start:index]))
            continue

        index += 1

    return literals


def format_specs(text: str) -> list[str]:
    return [match.group(0) for match in FORMAT_SPEC.finditer(text) if match.group(0) != "%%"]


def directive_ranges(source: str) -> list[tuple[int, int]]:
    """Return the character ranges covered by preprocessor directives."""
    ranges: list[tuple[int, int]] = []
    offset = 0
    for line in source.splitlines(keepends=True):
        stripped = line.lstrip()
        if stripped.startswith("#"):
            ranges.append((offset, offset + len(line)))
        offset += len(line)
    return ranges


def in_ranges(position: int, ranges: list[tuple[int, int]]) -> bool:
    for start, end in ranges:
        if start <= position < end:
            return True
    return False


def translate(
    source: str,
    table: dict[str, str],
    strict: bool,
) -> tuple[str, dict[str, int], list[str]]:
    """Return the rewritten source plus statistics and warnings."""
    stats = {"replaced": 0, "unmapped": 0, "skipped_wide": 0, "skipped_id": 0}
    warnings: list[str] = []
    pieces: list[str] = []
    cursor = 0
    unmapped: set[str] = set()
    directives = directive_ranges(source)

    for literal in find_literals(source):
        if literal.wide:
            stats["skipped_wide"] += 1
            continue
        if in_ranges(literal.start, directives):
            # Preprocessor directives carry file names, not user facing text.
            continue
        body = literal.body
        if body.startswith("##"):
            stats["skipped_id"] += 1
            continue
        replacement = table.get(body)
        if replacement is None or replacement == body:
            unmapped.add(body)
            continue
        if format_specs(body) != format_specs(replacement):
            warnings.append(
                "format specifier mismatch, left untranslated: "
                f"{body!r} -> {replacement!r}"
            )
            continue
        pieces.append(source[cursor:literal.start])
        pieces.append('"' + replacement + '"')
        cursor = literal.end
        stats["replaced"] += 1

    pieces.append(source[cursor:])
    stats["unmapped"] = len(unmapped)
    return "".join(pieces), stats, warnings


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--map", required=True, help="translation table (JSON)")
    parser.add_argument("--source", required=True, help="menu header to translate")
    parser.add_argument("--output", help="write the translated header here")
    parser.add_argument(
        "--in-place",
        action="store_true",
        help="overwrite the source file with the translated header",
    )
    parser.add_argument(
        "--report",
        action="store_true",
        help="only report statistics; do not write any file",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="exit non-zero when a translation had to be rejected",
    )
    parser.add_argument(
        "--list-unmapped",
        action="store_true",
        help="print the literals that have no translation",
    )
    args = parser.parse_args()

    table_path = Path(args.map)
    source_path = Path(args.source)

    table = json.loads(table_path.read_text(encoding="utf-8"))
    if not isinstance(table, dict):
        print(f"{table_path}: expected a JSON object", file=sys.stderr)
        return 2

    source = source_path.read_text(encoding="utf-8-sig")
    translated, stats, warnings = translate(source, table, args.strict)

    for warning in warnings:
        print(f"warning: {warning}", file=sys.stderr)

    print(
        f"{source_path}: replaced {stats['replaced']} literal(s), "
        f"{stats['unmapped']} unmapped, "
        f"{stats['skipped_wide']} wide literal(s) skipped, "
        f"{stats['skipped_id']} ImGui ID(s) skipped"
    )

    unused = sorted(set(table) - {
        literal.body
        for literal in find_literals(source)
        if not literal.wide and not literal.body.startswith("##")
    })
    if unused:
        print(
            f"{table_path}: {len(unused)} entr(y/ies) not present in the source "
            "(stale or not yet used)"
        )
        if args.list_unmapped:
            for key in unused:
                print(f"  stale: {key!r}")

    if args.list_unmapped:
        unmapped = sorted(
            literal.body
            for literal in find_literals(source)
            if not literal.wide
            and not literal.body.startswith("##")
            and literal.body not in table
        )
        for key in unmapped:
            print(f"  unmapped: {key!r}")

    if args.report:
        return 1 if (warnings and args.strict) else 0

    if args.in_place:
        # The Chinese build embeds non-ASCII literals, so the header is written
        # with a UTF-8 BOM: MSVC then treats it as UTF-8 regardless of the
        # active code page.
        source_path.write_text(translated, encoding="utf-8-sig")
        print(f"wrote {source_path}")
    elif args.output:
        output_path = Path(args.output)
        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(translated, encoding="utf-8-sig")
        print(f"wrote {output_path}")
    else:
        print("nothing written; pass --output or --in-place", file=sys.stderr)
        return 2

    return 1 if (warnings and args.strict) else 0


if __name__ == "__main__":
    raise SystemExit(main())
