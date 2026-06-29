# -*- coding: utf-8 -*-
"""
将 Terminal OutputBlocks 的 Lines 写入 DA_CrankItTerminalCommands（数据来自脚本内 STATIC_FALLBACK）。

用法（在 Unreal Editor 内）：
  1. 启用 Python Editor Script Plugin
  2. Tools → Execute Python Script → 选本文件
     或在 Output Log 的 Python 控制台：exec(open(r'D:/UE_works/CrankIt/Scripts/populate_terminal_output_blocks.py').read())

可选：修改下方 ASSET_PATH；留空则自动搜索名为 DA_CrankItTerminalCommands 的资产。

仅解析、不写 DA（可在系统 Python 运行）：
  python populate_terminal_output_blocks.py --parse-only
"""

from __future__ import annotations

import argparse
import os
import re
import sys
from typing import Dict, List, Optional

# ============ 配置 ============
ASSET_PATH = ""  # 例: "/Game/DataAsset/DA_CrankItTerminalCommands.DA_CrankItTerminalCommands"

# 相对本脚本：CrankIt/Scripts -> CrankIt/Source/CrankIt
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.normpath(os.path.join(SCRIPT_DIR, ".."))
SOURCE_DIR = os.path.join(PROJECT_ROOT, "Source", "CrankIt")

CPP_FILES = (
    "TerminalWidget.cpp",
    "ComputerScreenActor.cpp",
)

# CrankItNarrativeIds.h 中全部 Terminal Output Block（共 33 个）
ALL_BLOCK_IDS = (
    "ComputerScreen_Startup",
    "ComputerScreen_Destination",
    "ComputerScreen_FirstPrompt",
    "Boordle_GuessCorrectFollowup",
    "Boordle_OutOfGuessesPrefix",
    "ScanRepair_Finished",
    "Boordle_Started",
    "Calibrate_Started",
    "UpdateSystem_Warning",
    "LiftQuarantine_Warning",
    "CalibrateNorthEntry_Unlocking",
    "CalibrateNorthEntry_Unlocked",
    "CalibrateNorthEntry_LowAux",
    "CalibrateNorthEntry_Restricted",
    "CalibrateNorthEntry_Countdown30",
    "CalibrateNorthEntry_UnlockedReady",
    "GodIsDead",
    "LiftOperational",
    "Ascend_GameOver",
    "Classification_AllCorrect",
    "Classification_Ended",
    "Classification_BackToTerminal",
    "Classification_WidgetMissing",
    "Calibration_HumanVerified",
    "Calibration_Ended",
    "Calibration_BackToTerminal",
    "Calibration_WidgetMissing",
    "Battery_TransferInProgress",
    "Battery_TransferAlreadyActive",
    "Battery_LowPowerError",
    "Battery_TransferStarted",
    "Battery_TransferComplete",
    "Battery_TransferInterrupted",
)

# 无相邻 LEGACY 块时使用的原文（与迁移前 C++ AppendPendingLines 一致）
STATIC_FALLBACK: Dict[str, List[str]] = {
    "Boordle_GuessCorrectFollowup": [
        "",
        "Fault Successfully Fixed.",
        "",
        "Error!",
        "Unauthorised use of BOORDLE has been detected",
        "please verify you are human by typing the following:",
        '"I AM A HUMAN BEING"',
    ],
    "Boordle_OutOfGuessesPrefix": [
        ">>> Out of guesses. Target will refresh.",
    ],
    "ScanRepair_Finished": [
        "",
        "Scanning finished",
        "Finished FAULTS FOUND (1): ",
        "SystemTools.bin is malformed ",
        "Suggestion use BOORDLE to identify binary fault ",
    ],
    "Boordle_Started": [
        "Binary Guess Game Started",
    ],
    "Calibrate_Started": [
        "CALIBRATE Game Started",
    ],
    "UpdateSystem_Warning": [
        "",
        "Error!",
        "EMERGENCY QUARANTINE IS IN EFFECT",
        "",
        "to access lift controls quarantine must be lifted.",
        "all doors will open upon lifting quarantine.",
        "type the following to lift quarantine",
        '"LIFT QUARANTINE"',
    ],
    "LiftQuarantine_Warning": [
        "",
        "Error!",
        "EMERGENCY QUARANTINE IS IN EFFECT",
        "",
        "to access lift controls quarantine must be lifted.",
        "all doors will open upon lifting quarantine.",
        "type the following to lift quarantine",
        '"LIFT QUARANTINE"',
    ],
    "CalibrateNorthEntry_Unlocking": [
        "",
        "Unlocking Door Please wait..",
        "",
    ],
    "CalibrateNorthEntry_Unlocked": [
        "Door Unlocked thank you for being",
        "Patient",
    ],
    "CalibrateNorthEntry_LowAux": [
        "",
        "Error!",
        "Low auxiliary detected.",
        "A recharge is required to operate lift safely",
        "",
        "press any key to continue",
    ],
    "CalibrateNorthEntry_Restricted": [
        "",
        "Warning this terminal has been temporarily restricted",
        "to avoid excessive use, for YOUR SAFETY.",
        "",
        "Terminal will be available in 60 seconds",
    ],
    "CalibrateNorthEntry_Countdown30": [
        "Terminal will be available in 30 seconds",
        "",
    ],
    "CalibrateNorthEntry_UnlockedReady": [
        "Terminal unlocked press any key to continue",
    ],
    "GodIsDead": ["GOD IS DEAD"] * 14,
    "LiftOperational": [
        "",
        "lift operational",
        "to ascend type",
        '"ASCEND"',
    ],
    "Ascend_GameOver": [
        "GAME OVER!",
    ],
    "Classification_AllCorrect": [
        "All classifications correct.",
    ],
    "Classification_Ended": [
        "Classification game ended.",
    ],
    "Classification_BackToTerminal": [
        "Classification game finished. Back to terminal.",
    ],
    "Classification_WidgetMissing": [
        "ClassificationGameWidget is not bound. Please check widget name in UMG.",
    ],
    "Calibration_HumanVerified": [
        "",
        "HUMAN BEING verified",
        "",
        "Error!",
        "An update is required to continue",
        "",
        "type the following to start the update",
        '"UPDATE SYSTEM"',
    ],
    "Calibration_Ended": [
        "Calibration game finished.",
    ],
    "Calibration_BackToTerminal": [
        "Calibration game finished. Back to terminal.",
    ],
    "Calibration_WidgetMissing": [
        "CalibrationWidget is not bound. Please check widget name in UMG.",
    ],
    "Battery_TransferInProgress": [
        "Auxiliary power transfer in progress. Please wait...",
    ],
    "Battery_TransferAlreadyActive": [
        "Auxiliary transfer already in progress.",
    ],
    "Battery_LowPowerError": [
        "",
        "ERROR: Low auxiliary power.",
        "Insert powered batteries into TerminalSlot slots.",
        "",
    ],
    "Battery_TransferStarted": [
        "",
        "Auxiliary power transfer initiated.",
        "Do not remove batteries until transfer completes.",
    ],
    "Battery_TransferComplete": [
        "",
        "Auxiliary power transfer complete.",
        "Terminal input restored.",
        "",
    ],
    "Battery_TransferInterrupted": [
        "",
        "Auxiliary power transfer interrupted.",
        "Progress reset.",
        "",
        "ERROR: Low auxiliary power.",
        "Insert powered batteries into TerminalSlot slots.",
        "",
    ],
}

RE_APPEND_BLOCK = re.compile(
    r"AppendTerminal(?:Output)?Block\s*\([^,]+,\s*CrankItNarrative::Terminal::(\w+)\s*\)"
)
RE_STRING_LITERAL = re.compile(
    r'(?:TEXT\s*\(\s*|AddNewLine\s*\(\s*|AddNewLines\s*\(\s*)["]((?:\\.|[^"\\])*)["]'
)


def _unescape_cpp_string(s: str) -> str:
    return s.replace('\\"', '"').replace("\\n", "\n").replace("\\t", "\t")


def _extract_strings_from_legacy_body(body: str) -> List[str]:
    lines: List[str] = []
    for m in RE_STRING_LITERAL.finditer(body):
        lines.append(_unescape_cpp_string(m.group(1)))
    return lines


def _parse_legacy_named_blocks(content: str) -> Dict[str, List[str]]:
    """/* LEGACY BlockName ... AddNewLine("...") */"""
    result: Dict[str, List[str]] = {}
    pattern = re.compile(r"/\*\s*LEGACY\s+([A-Za-z0-9_]+)[^\*]*?\*/", re.DOTALL)
    for m in pattern.finditer(content):
        block_name = m.group(1)
        if block_name.startswith("Tutorial") or block_name.startswith("Intro"):
            continue
        body = m.group(0)
        strings = _extract_strings_from_legacy_body(body)
        if strings:
            result[block_name] = strings
    return result


def _parse_legacy_oneline_after_append(content: str) -> Dict[str, List[str]]:
    """AppendTerminalOutputBlock(...::Foo); 下一行 /* LEGACY: TEXT("...") */"""
    result: Dict[str, List[str]] = {}
    lines = content.splitlines()
    last_block: Optional[str] = None

    for i, line in enumerate(lines):
        append_m = RE_APPEND_BLOCK.search(line)
        if append_m:
            last_block = append_m.group(1)

        if last_block and "LEGACY" in line and "/*" in line:
            strings = _extract_strings_from_legacy_body(line)
            if strings:
                result[last_block] = strings
                last_block = None
                continue

        # 多行 LEGACY 紧跟在 Append 后（ComputerScreen_Startup 型）
        if last_block and "LEGACY" in line and "/*" in line:
            end = i
            while end < len(lines) and "*/" not in lines[end]:
                end += 1
            block_text = "\n".join(lines[i : end + 1])
            strings = _extract_strings_from_legacy_body(block_text)
            if strings:
                result[last_block] = strings
                last_block = None

    return result


def _parse_legacy_multiline_following_append(content: str) -> Dict[str, List[str]]:
    """Append...Block(...::Name); 随后 /* LEGACY Name ... */"""
    result: Dict[str, List[str]] = {}
    pattern = re.compile(
        r"AppendTerminal(?:Output)?Block\s*\([^,]+,\s*CrankItNarrative::Terminal::(\w+)\s*\)\s*;"
        r"[\s\S]*?"
        r"/\*\s*LEGACY\s+\1[^\*]*?\*/",
        re.MULTILINE,
    )
    for m in pattern.finditer(content):
        block_name = m.group(1)
        strings = _extract_strings_from_legacy_body(m.group(0))
        if strings:
            result[block_name] = strings
    return result


def parse_cpp_legacy(source_dir: str) -> Dict[str, List[str]]:
    merged: Dict[str, List[str]] = {}

    for filename in CPP_FILES:
        path = os.path.join(source_dir, filename)
        if not os.path.isfile(path):
            print(f"[warn] 找不到源文件: {path}")
            continue
        with open(path, "r", encoding="utf-8") as f:
            content = f.read()

        for parser in (
            _parse_legacy_named_blocks,
            _parse_legacy_oneline_after_append,
        ):
            merged.update(parser(content))

        multiline = _parse_legacy_multiline_following_append(content)
        merged.update(multiline)

    return merged


def build_output_blocks(source_dir: str) -> Dict[str, List[str]]:
    blocks = dict(STATIC_FALLBACK)
    parsed = parse_cpp_legacy(source_dir)
    blocks.update(parsed)

    # ComputerScreen 默认值（若 LEGACY 未解析到）
    blocks.setdefault(
        "ComputerScreen_Startup",
        ["Lift Operational ", "descending please wait "],
    )
    blocks.setdefault(
        "ComputerScreen_Destination",
        ["you have reached your destination. ", "have a nice day. "],
    )
    blocks.setdefault(
        "ComputerScreen_FirstPrompt",
        [
            "UNCAUGHT EXCEPTION",
            "<invalid mem address>",
            "SYSTEM CORRUPTION has occurred",
            "a REBOOT is required.",
        ],
    )

    return blocks


def validate_blocks(blocks: Dict[str, List[str]]) -> None:
    missing = [bid for bid in ALL_BLOCK_IDS if bid not in blocks]
    empty = [bid for bid in ALL_BLOCK_IDS if bid in blocks and not blocks[bid]]
    if missing:
        print(f"[warn] 缺少 BlockId ({len(missing)}): {', '.join(missing)}")
    if empty:
        print(f"[warn] Lines 为空 ({len(empty)}): {', '.join(empty)}")
    print(f"[info] 共 {len(blocks)} 个块，ALL_BLOCK_IDS 覆盖 {len(ALL_BLOCK_IDS) - len(missing)}/{len(ALL_BLOCK_IDS)}")


def find_asset_path() -> Optional[str]:
    try:
        import unreal  # type: ignore
    except ImportError:
        return None

    if ASSET_PATH:
        return ASSET_PATH

    candidates: List[str] = []

    # 优先用 Asset Registry
    try:
        registry = unreal.AssetRegistryHelpers.get_asset_registry()
        try:
            class_path = unreal.TopLevelAssetPath("/Script/CrankIt", "CrankItTerminalCommandData")
            asset_data_list = registry.get_assets_by_class(class_path, True)
        except (AttributeError, TypeError):
            # UE 4.x / 旧 API
            asset_data_list = registry.get_assets_by_class("CrankItTerminalCommandData", True)

        for asset_data in asset_data_list:
            name = str(asset_data.asset_name)
            if "DA_CrankItTerminalCommands" in name:
                path = str(asset_data.object_path) if hasattr(asset_data, "object_path") else asset_data.get_soft_object_path().path
                if path and not path.endswith("_C"):
                    candidates.append(path)
    except Exception as exc:
        unreal.log_warning(f"AssetRegistry 搜索失败: {exc}")  # type: ignore[name-defined]

    # 回退：仅在 /Game 下递归（不要用 /CrankIt，非 Content 根路径）
    if not candidates:
        try:
            for p in unreal.EditorAssetLibrary.list_assets("/Game", recursive=True):
                if "DA_CrankItTerminalCommands" in p and not p.endswith("_C"):
                    candidates.append(p)
        except Exception as exc:
            unreal.log_warning(f"list_assets(/Game) 失败: {exc}")  # type: ignore[name-defined]

    if not candidates:
        return None
    if len(candidates) == 1:
        return candidates[0]

    for p in candidates:
        if p.rsplit("/", 1)[-1].startswith("DA_CrankItTerminalCommands"):
            return p
    return candidates[0]


def _get_output_block_struct_class():
    """UE Python 暴露名为 CrankItTerminalOutputBlock（无 F 前缀）。"""
    import unreal  # type: ignore

    for attr in ("CrankItTerminalOutputBlock", "FCrankItTerminalOutputBlock"):
        if hasattr(unreal, attr):
            return getattr(unreal, attr)

    struct_obj = unreal.find_object(None, "/Script/CrankIt.CrankItTerminalOutputBlock")
    if struct_obj:
        return struct_obj

    raise RuntimeError(
        "找不到 CrankItTerminalOutputBlock。"
        "请先编译 C++ 并重启编辑器，确保 CrankIt 模块已加载。"
    )


def _make_output_block(lines: List[str]):
    import unreal  # type: ignore

    block_class = _get_output_block_struct_class()
    entry = block_class()
    entry.set_editor_property("Lines", list(lines))
    return entry


def _map_key_in_output_blocks(output_blocks, name_key) -> bool:
    try:
        if name_key in output_blocks:
            return True
    except TypeError:
        pass
    key_str = str(name_key)
    for k in output_blocks:
        if str(k) == key_str:
            return True
    return False


def apply_to_data_asset(blocks: Dict[str, List[str]], asset_path: str) -> None:
    import unreal  # type: ignore

    asset = unreal.load_asset(asset_path)
    if not asset:
        raise RuntimeError(f"无法加载资产: {asset_path}")

    try:
        if not isinstance(asset, unreal.CrankItTerminalCommandData):
            unreal.log_warning(  # type: ignore[name-defined]
                f"资产可能不是 CrankItTerminalCommandData: {asset_path}"
            )
    except AttributeError:
        pass

    block_struct = _get_output_block_struct_class()
    existing = asset.get_editor_property("OutputBlocks")

    # 复制已有 TMap
    new_map = unreal.Map(unreal.Name, block_struct)
    if existing:
        for key in existing:
            new_map[key] = existing[key]

    updated = 0
    created = 0
    skipped = 0

    for block_id in ALL_BLOCK_IDS:
        lines = blocks.get(block_id)
        if not lines:
            print(f"[skip] 无文案: {block_id}")
            skipped += 1
            continue

        name_key = unreal.Name(block_id)
        entry = _make_output_block(lines)

        if _map_key_in_output_blocks(new_map, name_key):
            updated += 1
        else:
            created += 1
        new_map[name_key] = entry

    asset.set_editor_property("OutputBlocks", new_map)
    asset.modify()
    unreal.EditorAssetLibrary.save_loaded_asset(asset)

    print(f"[done] 已写入 {asset_path}")
    print(f"       更新 {updated} 个键，新建 {created} 个键，跳过 {skipped} 个")


def main_standalone(parse_only: bool = False) -> int:
    blocks = build_output_blocks(SOURCE_DIR)
    validate_blocks(blocks)

    if parse_only:
        print("\n--- 解析结果预览 ---")
        for block_id in ALL_BLOCK_IDS:
            lines = blocks.get(block_id, [])
            preview = " | ".join(lines[:3])
            if len(lines) > 3:
                preview += f" | ... (+{len(lines) - 3} lines)"
            print(f"  {block_id} ({len(lines)}): {preview}")
        return 0

    if "unreal" not in sys.modules:
        try:
            import unreal  # noqa: F401
        except ImportError:
            print(
                "\n[error] 未检测到 unreal 模块。请在 Unreal Editor 内执行本脚本，"
                "或使用: python populate_terminal_output_blocks.py --parse-only"
            )
            return 1

    asset_path = find_asset_path()
    if not asset_path:
        print("[error] 未找到 DA_CrankItTerminalCommands，请设置 ASSET_PATH")
        return 1

    apply_to_data_asset(blocks, asset_path)
    return 0


if __name__ == "__main__":
    arg_parser = argparse.ArgumentParser(description="填充 DA_CrankItTerminalCommands OutputBlocks")
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

        blocks = build_output_blocks(SOURCE_DIR)
        validate_blocks(blocks)
        asset_path = find_asset_path()
        if not asset_path:
            print("[error] 未找到 DA_CrankItTerminalCommands，请在本脚本顶部设置 ASSET_PATH")
            raise SystemExit(1)
        apply_to_data_asset(blocks, asset_path)
    except ImportError:
        raise SystemExit(main_standalone(parse_only=False))
