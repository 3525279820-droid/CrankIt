# -*- coding: utf-8 -*-
"""
将字幕轨 Lines 写入 DA_CrankItNarrative（数据来自脚本内 STATIC_FALLBACK）。

用法（Unreal Editor 内）：
  Tools → Execute Python Script → 选本文件

可选：修改 ASSET_PATH；留空则自动搜索 DA_CrankItNarrative。

仅解析预览（系统 Python）：
  python populate_narrative_subtitle_tracks.py --parse-only
"""

from __future__ import annotations

import argparse
import os
import re
import sys
from typing import Any, Dict, List, Optional

# ============ 配置 ============
ASSET_PATH = ""  # 例: "/Game/DataAsset/DA_CrankItNarrative.DA_CrankItNarrative"

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.normpath(os.path.join(SCRIPT_DIR, ".."))
SOURCE_DIR = os.path.join(PROJECT_ROOT, "Source", "CrankIt")

CPP_FILES = (
    "InitLevel.cpp",
    "MineConsole.cpp",
)

# CrankItNarrativeIds.h → CrankItNarrative::Subtitle（共 8 条轨）
ALL_TRACK_IDS = (
    "Intro_SkipTutorialPrompt",
    "TutorialSkipped",
    "TutorialNotSkipped_GordonIntro",
    "PostTutorial",
    "ChargeTutorial_Level1",
    "ChargeTutorial_Level2",
    "ChargeTutorial_Level3",
    "EMP_Tutorial",
)

# LEGACY 注释名 → TrackId（ChargeTutorial 在 LEGACY 内按 case 拆分）
LEGACY_NAME_TO_TRACK = {
    "Intro_SkipTutorialPrompt": "Intro_SkipTutorialPrompt",
    "TutorialSkipped": "TutorialSkipped",
    "TutorialNotSkipped_GordonIntro": "TutorialNotSkipped_GordonIntro",
    "PostTutorial": "PostTutorial",
    "EMP_Tutorial": "EMP_Tutorial",
}

# 解析失败时的完整 fallback（与迁移前 C++ 硬编码一致）
STATIC_FALLBACK: Dict[str, List[Dict[str, Any]]] = {
    "Intro_SkipTutorialPrompt": [
        {"StartTimeSeconds": 0.0, "EndTimeSeconds": 2.5, "Text": "Ah, you must be the new guy."},
        {"StartTimeSeconds": 2.5, "EndTimeSeconds": 5.0, "Text": "Do you have any idea what you're doing?"},
    ],
    "TutorialSkipped": [
        {"StartTimeSeconds": 0.0, "EndTimeSeconds": 2.5, "Text": "【字幕测试】教程已跳过"},
    ],
    "TutorialNotSkipped_GordonIntro": [
        {"StartTimeSeconds": 0.0, "EndTimeSeconds": 2.5, "Text": "Hahaha, me neither!"},
        {"StartTimeSeconds": 2.5, "EndTimeSeconds": 3.5, "Text": "The air down here is as thin as my wallet."},
        {"StartTimeSeconds": 3.5, "EndTimeSeconds": 5.0, "Text": "Ha!"},
        {"StartTimeSeconds": 5.0, "EndTimeSeconds": 7.5, "Text": "Just kidding. I'm Gordon."},
        {"StartTimeSeconds": 7.5, "EndTimeSeconds": 10.0, "Text": "I'm a highly trained professional."},
        {"StartTimeSeconds": 10.0, "EndTimeSeconds": 12.5, "Text": "Uh, Anyways..."},
        {
            "StartTimeSeconds": 12.5,
            "EndTimeSeconds": 14.5,
            "Text": "I'm gonna need you to use this here state of the art machinery.",
        },
        {
            "StartTimeSeconds": 14.5,
            "EndTimeSeconds": 16.5,
            "Text": "'The EMP Light Manifold System'\n(All Right Reserved)",
        },
        {"StartTimeSeconds": 16.5, "EndTimeSeconds": 18.5, "Text": "to blast the western cave with the luminescene."},
        {"StartTimeSeconds": 18.5, "EndTimeSeconds": 20.5, "Text": "Charge a battery, and send it!"},
    ],
    "PostTutorial": [
        {"StartTimeSeconds": 0.0, "EndTimeSeconds": 2.5, "Text": "【字幕测试】教程测试文本2"},
    ],
    "ChargeTutorial_Level1": [
        {"StartTimeSeconds": 0.0, "EndTimeSeconds": 2.5, "Text": "That's it! Crank it harder!"},
    ],
    "ChargeTutorial_Level2": [
        {"StartTimeSeconds": 0.0, "EndTimeSeconds": 2.5, "Text": "I can tell you've done this before!"},
    ],
    "ChargeTutorial_Level3": [
        {"StartTimeSeconds": 0.0, "EndTimeSeconds": 2.5, "Text": "I bet the boys upstairs love you!"},
    ],
    "EMP_Tutorial": [
        {"StartTimeSeconds": 0.0, "EndTimeSeconds": 2.5, "Text": "Argh, you stupid fucking idiot!"},
        {"StartTimeSeconds": 2.5, "EndTimeSeconds": 3.5, "Text": "You almost blinded me!"},
        {"StartTimeSeconds": 3.5, "EndTimeSeconds": 4.0, "Text": "Just kidding."},
        {"StartTimeSeconds": 4.0, "EndTimeSeconds": 5.0, "Text": "I'm Gordon."},
        {"StartTimeSeconds": 5.0, "EndTimeSeconds": 6.0, "Text": "The light doesn't bother me."},
        {"StartTimeSeconds": 6.0, "EndTimeSeconds": 7.0, "Text": "I'm sorta just built different."},
        {"StartTimeSeconds": 7.0, "EndTimeSeconds": 8.0, "Text": "Oh, by the way,"},
        {
            "StartTimeSeconds": 8.0,
            "EndTimeSeconds": 9.0,
            "Text": "You can use your decibel meter tocheck for sounds in the tunnels.",
        },
        {"StartTimeSeconds": 9.0, "EndTimeSeconds": 10.0, "Text": "It picks up even the smallest of movements!"},
        {"StartTimeSeconds": 10.0, "EndTimeSeconds": 10.5, "Text": "Hmm..."},
        {"StartTimeSeconds": 10.5, "EndTimeSeconds": 11.5, "Text": "That's Strange!"},
        {
            "StartTimeSeconds": 11.5,
            "EndTimeSeconds": 13.5,
            "Text": "I'm getting some incredibly largeseismic activity down there!",
        },
        {"StartTimeSeconds": 13.5, "EndTimeSeconds": 14.5, "Text": "Maybe it's your mom?"},
        {"StartTimeSeconds": 14.5, "EndTimeSeconds": 15.0, "Text": "Ha!"},
        {"StartTimeSeconds": 15.0, "EndTimeSeconds": 16.5, "Text": "Mm, ima go check it out."},
        {"StartTimeSeconds": 16.5, "EndTimeSeconds": 18.5, "Text": "See you later cranker!"},
        {"StartTimeSeconds": 18.5, "EndTimeSeconds": 20.5, "Text": "[unintelligible]"},
    ],
}

RE_SUBTITLE_CUE = re.compile(
    r"\{(\d+(?:\.\d+)?)\s*f?\s*,\s*(\d+(?:\.\d+)?)\s*f?\s*,\s*TEXT\s*\(\s*((?:\"(?:\\.|[^\"\\])*\"\s*)+)\s*\)\s*\}",
    re.DOTALL,
)
RE_CPP_STRING = re.compile(r"\"((?:\\.|[^\"\\])*)\"")


def _unescape_cpp_string(s: str) -> str:
    return (
        s.replace('\\"', '"')
        .replace("\\n", "\n")
        .replace("\\t", "\t")
        .replace("\\'", "'")
    )


def _concat_text_macro_strings(text_macro_args: str) -> str:
    parts = RE_CPP_STRING.findall(text_macro_args)
    return _unescape_cpp_string("".join(parts))


def _parse_subtitle_cues_from_text(text: str) -> List[Dict[str, Any]]:
    cues: List[Dict[str, Any]] = []
    for m in RE_SUBTITLE_CUE.finditer(text):
        cues.append(
            {
                "StartTimeSeconds": float(m.group(1)),
                "EndTimeSeconds": float(m.group(2)),
                "Text": _concat_text_macro_strings(m.group(3)),
            }
        )
    return cues


def _parse_charge_tutorial_legacy(body: str) -> Dict[str, List[Dict[str, Any]]]:
    result: Dict[str, List[Dict[str, Any]]] = {}
    for level in (1, 2, 3):
        case_m = re.search(
            rf"case\s+{level}\s*:\s*Lines\s*=\s*\{{([^}}]+)\}}",
            body,
            re.DOTALL,
        )
        if not case_m:
            continue
        cues = _parse_subtitle_cues_from_text(case_m.group(1))
        if cues:
            result[f"ChargeTutorial_Level{level}"] = cues
    return result


def _parse_legacy_named_subtitle_blocks(content: str) -> Dict[str, List[Dict[str, Any]]]:
    result: Dict[str, List[Dict[str, Any]]] = {}
    pattern = re.compile(r"/\*\s*LEGACY\s+(\w+)[^\*]*?\*/", re.DOTALL)
    for m in pattern.finditer(content):
        legacy_name = m.group(1)
        body = m.group(0)

        if legacy_name == "ChargeTutorial":
            result.update(_parse_charge_tutorial_legacy(body))
            continue

        track_id = LEGACY_NAME_TO_TRACK.get(legacy_name, legacy_name)
        cues = _parse_subtitle_cues_from_text(body)
        if cues:
            result[track_id] = cues
    return result


def parse_cpp_legacy(source_dir: str) -> Dict[str, List[Dict[str, Any]]]:
    merged: Dict[str, List[Dict[str, Any]]] = {}
    for filename in CPP_FILES:
        path = os.path.join(source_dir, filename)
        if not os.path.isfile(path):
            print(f"[warn] 找不到源文件: {path}")
            continue
        with open(path, "r", encoding="utf-8") as f:
            content = f.read()
        merged.update(_parse_legacy_named_subtitle_blocks(content))
    return merged


def build_subtitle_tracks(source_dir: str) -> Dict[str, List[Dict[str, Any]]]:
    tracks = {k: [dict(c) for c in v] for k, v in STATIC_FALLBACK.items()}
    parsed = parse_cpp_legacy(source_dir)
    for track_id, cues in parsed.items():
        fallback = tracks.get(track_id, [])
        if fallback and len(cues) < len(fallback):
            print(
                f"[warn] LEGACY 解析 {track_id} 仅 {len(cues)}/{len(fallback)} 条，使用 STATIC_FALLBACK"
            )
            continue
        tracks[track_id] = cues
    return tracks


def validate_tracks(tracks: Dict[str, List[Dict[str, Any]]]) -> None:
    missing = [tid for tid in ALL_TRACK_IDS if tid not in tracks]
    empty = [tid for tid in ALL_TRACK_IDS if tid in tracks and not tracks[tid]]
    if missing:
        print(f"[warn] 缺少 TrackId ({len(missing)}): {', '.join(missing)}")
    if empty:
        print(f"[warn] Lines 为空 ({len(empty)}): {', '.join(empty)}")
    print(
        f"[info] 共 {len(tracks)} 条轨，ALL_TRACK_IDS 覆盖 "
        f"{len(ALL_TRACK_IDS) - len(missing)}/{len(ALL_TRACK_IDS)}"
    )


def find_asset_path() -> Optional[str]:
    try:
        import unreal  # type: ignore
    except ImportError:
        return None

    if ASSET_PATH:
        return ASSET_PATH

    candidates: List[str] = []

    try:
        registry = unreal.AssetRegistryHelpers.get_asset_registry()
        try:
            class_path = unreal.TopLevelAssetPath("/Script/CrankIt", "CrankItNarrativeData")
            asset_data_list = registry.get_assets_by_class(class_path, True)
        except (AttributeError, TypeError):
            asset_data_list = registry.get_assets_by_class("CrankItNarrativeData", True)

        for asset_data in asset_data_list:
            name = str(asset_data.asset_name)
            if "DA_CrankItNarrative" in name:
                path = (
                    str(asset_data.object_path)
                    if hasattr(asset_data, "object_path")
                    else asset_data.get_soft_object_path().path
                )
                if path and not path.endswith("_C"):
                    candidates.append(path)
    except Exception as exc:
        unreal.log_warning(f"AssetRegistry 搜索失败: {exc}")  # type: ignore[name-defined]

    if not candidates:
        try:
            for p in unreal.EditorAssetLibrary.list_assets("/Game", recursive=True):
                if "DA_CrankItNarrative" in p and not p.endswith("_C"):
                    candidates.append(p)
        except Exception as exc:
            unreal.log_warning(f"list_assets(/Game) 失败: {exc}")  # type: ignore[name-defined]

    if not candidates:
        return None
    if len(candidates) == 1:
        return candidates[0]

    for p in candidates:
        if p.rsplit("/", 1)[-1].startswith("DA_CrankItNarrative"):
            return p
    return candidates[0]


def _get_struct_class(*candidates: str):
    import unreal  # type: ignore

    for name in candidates:
        if hasattr(unreal, name):
            return getattr(unreal, name)

    script_path = "/Script/CrankIt." + candidates[0].lstrip("F")
    struct_obj = unreal.find_object(None, script_path)
    if struct_obj:
        return struct_obj

    raise RuntimeError(f"找不到 struct: {candidates}（请先编译 C++ 并重启编辑器）")


def _make_subtitle_line(cue: Dict[str, Any]):
    import unreal  # type: ignore

    line_class = _get_struct_class("CrankItSubtitleLine", "FCrankItSubtitleLine")
    line = line_class()
    line.set_editor_property("StartTimeSeconds", float(cue["StartTimeSeconds"]))
    line.set_editor_property("EndTimeSeconds", float(cue["EndTimeSeconds"]))
    line.set_editor_property("Text", str(cue["Text"]))
    return line


def _make_track_entry(track_id: str, cues: List[Dict[str, Any]]):
    import unreal  # type: ignore

    entry_class = _get_struct_class("CrankItSubtitleTrackEntry", "FCrankItSubtitleTrackEntry")
    entry = entry_class()
    entry.set_editor_property("TrackId", unreal.Name(track_id))
    lines = [_make_subtitle_line(c) for c in cues]
    entry.set_editor_property("Lines", lines)
    return entry


def _track_id_from_entry(entry) -> str:
    track_id = entry.get_editor_property("TrackId")
    return str(track_id)


def apply_to_data_asset(tracks: Dict[str, List[Dict[str, Any]]], asset_path: str) -> None:
    import unreal  # type: ignore

    asset = unreal.load_asset(asset_path)
    if not asset:
        raise RuntimeError(f"无法加载资产: {asset_path}")

    try:
        if not isinstance(asset, unreal.CrankItNarrativeData):
            unreal.log_warning(f"资产可能不是 CrankItNarrativeData: {asset_path}")  # type: ignore[name-defined]
    except AttributeError:
        pass

    existing = asset.get_editor_property("SubtitleTracks") or []
    by_id: Dict[str, Any] = {}
    order: List[str] = []

    for entry in existing:
        tid = _track_id_from_entry(entry)
        if tid and tid not in by_id:
            order.append(tid)
        by_id[tid] = entry

    updated = 0
    created = 0
    skipped = 0

    for track_id in ALL_TRACK_IDS:
        cues = tracks.get(track_id)
        if not cues:
            print(f"[skip] 无字幕: {track_id}")
            skipped += 1
            continue

        lines = [_make_subtitle_line(c) for c in cues]

        if track_id in by_id:
            entry = by_id[track_id]
            entry.set_editor_property("TrackId", unreal.Name(track_id))
            entry.set_editor_property("Lines", lines)
            updated += 1
        else:
            by_id[track_id] = _make_track_entry(track_id, cues)
            order.append(track_id)
            created += 1

    new_tracks = [by_id[tid] for tid in order if tid in by_id]

    asset.set_editor_property("SubtitleTracks", new_tracks)
    asset.modify()
    unreal.EditorAssetLibrary.save_loaded_asset(asset)

    print(f"[done] 已写入 {asset_path}")
    print(f"       更新 {updated} 条轨，新建 {created} 条轨，跳过 {skipped} 条")


def main_standalone(parse_only: bool = False) -> int:
    tracks = build_subtitle_tracks(SOURCE_DIR)
    validate_tracks(tracks)

    if parse_only:
        print("\n--- 解析结果预览 ---")
        for track_id in ALL_TRACK_IDS:
            cues = tracks.get(track_id, [])
            print(f"  {track_id} ({len(cues)} cues):")
            for cue in cues:
                text_preview = cue["Text"].replace("\n", "\\n")
                if len(text_preview) > 60:
                    text_preview = text_preview[:57] + "..."
                print(
                    f"    [{cue['StartTimeSeconds']:.1f} ~ {cue['EndTimeSeconds']:.1f}] {text_preview}"
                )
        return 0

    if "unreal" not in sys.modules:
        try:
            import unreal  # noqa: F401
        except ImportError:
            print(
                "\n[error] 未检测到 unreal 模块。请在 Unreal Editor 内执行，"
                "或使用 --parse-only"
            )
            return 1

    asset_path = find_asset_path()
    if not asset_path:
        print("[error] 未找到 DA_CrankItNarrative，请设置 ASSET_PATH")
        return 1

    apply_to_data_asset(tracks, asset_path)
    return 0


if __name__ == "__main__":
    arg_parser = argparse.ArgumentParser(description="填充 DA_CrankItNarrative SubtitleTracks")
    arg_parser.add_argument(
        "--parse-only",
        action="store_true",
        help="仅解析 C++ LEGACY，不写入 Data Asset",
    )
    args = arg_parser.parse_args()

    if args.parse_only:
        raise SystemExit(main_standalone(parse_only=True))

    try:
        import unreal  # noqa: F401

        tracks = build_subtitle_tracks(SOURCE_DIR)
        validate_tracks(tracks)
        asset_path = find_asset_path()
        if not asset_path:
            print("[error] 未找到 DA_CrankItNarrative，请在本脚本顶部设置 ASSET_PATH")
            raise SystemExit(1)
        apply_to_data_asset(tracks, asset_path)
    except ImportError:
        raise SystemExit(main_standalone(parse_only=False))
