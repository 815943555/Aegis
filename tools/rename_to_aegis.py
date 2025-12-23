#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
批量把框架对外标识符改为 Aegis 前缀（C89 项目用）。

规则（默认）：
- 类型：对 framework/include 中定义的 public typedef 名称，统一加前缀 `Aegis`。
- 函数：对一组已知层级前缀的函数名，统一改为 `aegis_<原前缀>...`。
- 为避免破坏头文件路径，默认跳过 `#include "xxx.h"` 行。

注意：这是一次性迁移工具；跑完后请构建 + 运行约束检查。
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path


DEFAULT_DIRS = [
    "framework",
    "application",
    "examples",
    "tests",
    "include",
    "src",
    "port",
]

# 仅对标识符（而不是文件名）做改名，所以要避免替换 include 行里的 "app_xxx.h" 等字符串。
INCLUDE_LINE = re.compile(r'^\s*#\s*include\s+"[^"]+"')

# 函数前缀：按长度从长到短替换，防止 `app_` 抢先匹配 `app_cmd_`。
FUNC_PREFIXES = [
    "entry_platform_",
    "infrastructure_",
    "app_cmd_service_",
    "app_cmd_",
    "app_query_",
    "app_init_",
    "app_dto_",
    "app_asm_",
    "app_conv_",
    "app_",
    "domain_",
    "entry_",
    "mem_pool_",
    "ring_buffer_",
    "trace_",
    "error_code_",
    "critical_",
    "hal_",
]


def collect_public_types(framework_include: Path) -> list[str]:
    types: set[str] = set()
    for header in framework_include.rglob("*.h"):
        text = header.read_text(encoding="utf-8", errors="ignore")
        # typedef struct/enum { ... } Name;
        for m in re.finditer(
            r"typedef\s+(?:struct|enum)\s*\{.*?\}\s*([A-Z][A-Za-z0-9_]*)\s*;",
            text,
            flags=re.S,
        ):
            types.add(m.group(1))
        # typedef xxx Name;
        for m in re.finditer(
            r"^\s*typedef\s+[^;\n]+\s+([A-Z][A-Za-z0-9_]*)\s*;\s*$",
            text,
            flags=re.M,
        ):
            types.add(m.group(1))
    return sorted(types, key=len, reverse=True)


def rewrite_file(path: Path, type_re: re.Pattern[str], prefix_re: re.Pattern[str]) -> bool:
    original = path.read_text(encoding="utf-8", errors="ignore").splitlines(True)
    changed = False
    out_lines: list[str] = []

    for line in original:
        if INCLUDE_LINE.match(line):
            out_lines.append(line)
            continue

        new_line = type_re.sub(lambda m: "Aegis" + m.group(1), line)
        new_line2 = prefix_re.sub(lambda m: "aegis_" + m.group(1), new_line)
        if new_line2 != line:
            changed = True
        out_lines.append(new_line2)

    if changed:
        path.write_text("".join(out_lines), encoding="utf-8")
    return changed


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".", help="仓库根目录")
    ap.add_argument("--dirs", nargs="*", default=DEFAULT_DIRS, help="需要处理的目录")
    args = ap.parse_args()

    root = Path(args.root).resolve()
    framework_include = root / "framework" / "include"
    if not framework_include.exists():
        raise SystemExit(f"找不到 {framework_include}")

    public_types = collect_public_types(framework_include)
    if not public_types:
        raise SystemExit("未收集到任何 public typedef 类型")

    type_re = re.compile(r"\b(" + "|".join(map(re.escape, public_types)) + r")\b")
    prefix_re = re.compile(r"\b(" + "|".join(map(re.escape, FUNC_PREFIXES)) + r")")

    changed_files = 0
    for d in args.dirs:
        base = (root / d)
        if not base.exists():
            continue
        for ext in (".c", ".h"):
            for p in base.rglob(f"*{ext}"):
                if "build" in p.parts:
                    continue
                if rewrite_file(p, type_re, prefix_re):
                    changed_files += 1

    print(f"✅ 完成：改写 {changed_files} 个文件")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

