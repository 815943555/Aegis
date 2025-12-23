#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
禁止可变全局变量检查脚本
用于强制“严格依赖注入/无隐藏状态”的约束：在指定目录内禁止定义非const的文件级变量。

作者: jack liu
"""

import os
import re
import sys
from pathlib import Path


VAR_DECL_RE = re.compile(
    r'^\s*(static\s+)?(const\s+)?'
    r'[A-Za-z_][A-Za-z0-9_]*'                 # base type or qualifier (simplified)
    r'(?:\s+[*A-Za-z_][A-Za-z0-9_\s\*]*)?'     # optional pointers / extra tokens
    r'\s+([A-Za-z_][A-Za-z0-9_]*)'             # var name
    r'\s*(?:\[[^\]]*\])?\s*'                  # optional array
    r'(?:=\s*[^;]+)?;'                        # optional init + semicolon
)


def strip_comments(line: str) -> str:
    # very small helper: remove // comments and trailing /* ... */ on the same line
    if '//' in line:
        line = line.split('//', 1)[0]
    if '/*' in line and '*/' in line:
        before = line.split('/*', 1)[0]
        after = line.split('*/', 1)[1]
        line = before + after
    return line


def check_file(path: Path, errors: list) -> None:
    try:
        content = path.read_text(encoding='utf-8', errors='ignore')
    except Exception:
        return

    brace_depth = 0
    in_block_comment = False
    lines = content.splitlines()

    for idx, raw in enumerate(lines, start=1):
        line = raw

        # block comment state machine
        if in_block_comment:
            if '*/' in line:
                in_block_comment = False
                line = line.split('*/', 1)[1]
            else:
                continue

        if '/*' in line and '*/' not in line:
            in_block_comment = True
            line = line.split('/*', 1)[0]

        line = strip_comments(line).strip()
        if not line:
            continue

        # update brace depth (rough)
        brace_depth += line.count('{')
        brace_depth -= line.count('}')
        if brace_depth != 0:
            continue

        # only check file-scope statements
        if line.startswith('#'):
            continue
        if line.startswith('typedef '):
            continue
        if line.startswith('extern '):
            continue
        if '(' in line:
            # function prototype / macro-like declarations
            continue
        if not line.endswith(';'):
            continue

        m = VAR_DECL_RE.match(line)
        if not m:
            continue

        is_const = m.group(2) is not None
        var_name = m.group(3)

        if not is_const:
            errors.append(f"{path}:{idx}: 可变全局变量 '{var_name}'（请改为注入/实例字段或标记为const）")


def main() -> int:
    if len(sys.argv) < 2:
        print("用法: python3 check_no_globals.py <代码根目录>")
        return 1

    root_dir = Path(sys.argv[1])
    if not root_dir.exists():
        print(f"错误: 目录不存在: {root_dir}")
        return 1

    errors = []

    for file_path in root_dir.rglob("*.c"):
        p = str(file_path)
        if 'build' in p:
            continue
        if os.sep + 'port' + os.sep in p:
            # port 层允许存在必要的全局状态（硬件寄存器/单例驱动等）
            continue
        check_file(file_path, errors)

    if errors:
        print("❌ 发现可变全局变量（违反严格依赖注入约束）:")
        for e in errors[:50]:
            print(e)
        if len(errors) > 50:
            print(f"... 还有 {len(errors) - 50} 个错误")
        return 1

    print("✅ 未发现可变全局变量")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

