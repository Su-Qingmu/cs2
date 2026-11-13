#!/usr/bin/env python3
"""Regression tests for scripts/translate_menu.py.

The Chinese binary is produced by rewriting the string literals of menu.hpp.
These tests pin the guarantees the translator has to keep:

* narrow literals are translated;
* wide literals (``L"..."``) are never touched, so persistence keys survive;
* ImGui IDs (``##``) are never touched;
* comments and preprocessor directives are never touched;
* a translation whose printf specifiers do not match the source is rejected.

Run with::

    python3 tests/translate_menu_tests.py
"""

from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT / "scripts"))

from translate_menu import find_literals, format_specs, translate  # noqa: E402

MENU = REPO_ROOT / "external-cheat-base" / "src" / "features" / "menu.hpp"
MAP = REPO_ROOT / "i18n" / "zh_map.json"


class TranslateMenuTests(unittest.TestCase):
    def test_replaces_narrow_literal(self) -> None:
        source = 'ImGui::Checkbox("Player ESP", &espEnabled);\n'
        table = {"Player ESP": "玩家 ESP"}
        result, stats, warnings = translate(source, table, strict=False)
        self.assertEqual(result, 'ImGui::Checkbox("玩家 ESP", &espEnabled);\n')
        self.assertEqual(stats["replaced"], 1)
        self.assertEqual(warnings, [])

    def test_keeps_wide_literal(self) -> None:
        source = 'readPersistentInt(L"web_radar_port", webRadarPort, path),\n'
        table = {"web_radar_port": "端口"}
        result, stats, _ = translate(source, table, strict=False)
        self.assertEqual(result, source)
        self.assertEqual(stats["skipped_wide"], 1)
        self.assertEqual(stats["replaced"], 0)

    def test_keeps_imgui_id(self) -> None:
        source = 'ImGui::Text("##QuickControls");\n'
        table = {"##QuickControls": "##快捷控制"}
        result, stats, _ = translate(source, table, strict=False)
        self.assertEqual(result, source)
        self.assertEqual(stats["skipped_id"], 1)

    def test_keeps_comments(self) -> None:
        source = '// the global "suppress until release" guard is set.\n'
        table = {"suppress until release": "抑制直到松开"}
        result, stats, _ = translate(source, table, strict=False)
        self.assertEqual(result, source)
        self.assertEqual(stats["replaced"], 0)

    def test_keeps_include_directives(self) -> None:
        source = '#include "features/menu.hpp"\n'
        table = {"features/menu.hpp": "features/menu_zh.hpp"}
        result, stats, _ = translate(source, table, strict=False)
        self.assertEqual(result, source)
        self.assertEqual(stats["replaced"], 0)

    def test_rejects_format_mismatch(self) -> None:
        source = 'ImGui::Text("Frames: %llu sent", sent);\n'
        table = {"Frames: %llu sent": "帧数：已发送"}
        result, stats, warnings = translate(source, table, strict=False)
        self.assertEqual(result, source)
        self.assertEqual(stats["replaced"], 0)
        self.assertEqual(len(warnings), 1)
        self.assertIn("format specifier mismatch", warnings[0])

    def test_format_specs_ignores_percent_escape(self) -> None:
        self.assertEqual(format_specs("100%% done"), [])
        self.assertEqual(format_specs("%s and %d"), ["%s", "%d"])

    def test_find_literals_ignores_comments(self) -> None:
        source = 'a("x"); // "y"\n/* "z" */\n'
        bodies = [literal.body for literal in find_literals(source)]
        self.assertEqual(bodies, ["x"])

    def test_repository_menu_only_changes_inside_literals(self) -> None:
        table = json.loads(MAP.read_text(encoding="utf-8"))
        source = MENU.read_text(encoding="utf-8")
        translated, stats, warnings = translate(source, table, strict=True)
        self.assertEqual(warnings, [], "translation table produced warnings")
        self.assertGreater(stats["replaced"], 100)

        def skeleton(text: str) -> str:
            pieces: list[str] = []
            cursor = 0
            for literal in find_literals(text):
                pieces.append(text[cursor:literal.start])
                pieces.append("<LIT>")
                cursor = literal.end
            pieces.append(text[cursor:])
            return "".join(pieces)

        self.assertEqual(skeleton(source), skeleton(translated))

        wide = [lit.body for lit in find_literals(source) if lit.wide]
        self.assertTrue(wide, "the menu is expected to contain wide literals")
        for body in wide:
            self.assertIn(body, translated)

    def test_translation_is_idempotent(self) -> None:
        table = json.loads(MAP.read_text(encoding="utf-8"))
        source = MENU.read_text(encoding="utf-8")
        once, _, _ = translate(source, table, strict=False)
        twice, stats, _ = translate(once, table, strict=False)
        self.assertEqual(once, twice)
        self.assertEqual(stats["replaced"], 0)

    def test_every_translation_keeps_format_specifiers(self) -> None:
        table = json.loads(MAP.read_text(encoding="utf-8"))
        for key, value in table.items():
            self.assertEqual(
                format_specs(key),
                format_specs(value),
                f"format specifier mismatch for {key!r}",
            )

    def test_temporary_output_round_trip(self) -> None:
        table = json.loads(MAP.read_text(encoding="utf-8"))
        source = MENU.read_text(encoding="utf-8")
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / "menu_zh.hpp"
            out.write_text(translate(source, table, strict=False)[0], encoding="utf-8")
            self.assertTrue(out.exists())
            self.assertIn("玩家 ESP", out.read_text(encoding="utf-8"))

    def test_written_header_carries_a_utf8_bom(self) -> None:
        # MSVC only treats a source file as UTF-8 when it has a BOM or the
        # /utf-8 compiler flag. The generated Chinese header must carry the
        # BOM so the build never depends on the active code page.
        table = json.loads(MAP.read_text(encoding="utf-8"))
        source = MENU.read_text(encoding="utf-8")
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / "menu_zh.hpp"
            out.write_text(
                translate(source, table, strict=False)[0], encoding="utf-8-sig"
            )
            self.assertTrue(out.read_bytes().startswith(b"\xef\xbb\xbf"))
            self.assertIn("玩家 ESP", out.read_text(encoding="utf-8-sig"))


if __name__ == "__main__":
    unittest.main(verbosity=2)
