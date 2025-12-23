#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
追溯信息检查脚本
检查ISO 26262追溯注释是否完整(@req, @design, @asil)

作者: jack liu
"""

import os
import re
import sys
from pathlib import Path


class TraceabilityChecker:
    """追溯性检查器"""

    def __init__(self, root_dir):
        self.root_dir = Path(root_dir)
        self.errors = []
        self.warnings = []
        self.checked_files = 0
        self.checked_functions = 0

    def check_file(self, file_path):
        """检查单个文件"""
        self.checked_files += 1

        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()

        # 检查文件级追溯注释
        self._check_file_traceability(content, file_path)

        # 检查函数级追溯注释
        self._check_function_traceability(content, file_path)

    def _check_file_traceability(self, content, file_path):
        """检查文件级追溯注释"""
        lines = content.split('\n')

        # 在文件头部查找追溯注释(前20行)
        header_content = '\n'.join(lines[:20])

        has_req = '@req:' in header_content or '@req ' in header_content
        has_design = '@design:' in header_content or '@design ' in header_content
        has_asil = '@asil:' in header_content or '@asil ' in header_content

        if not (has_req and has_design and has_asil):
            missing = []
            if not has_req:
                missing.append('@req')
            if not has_design:
                missing.append('@design')
            if not has_asil:
                missing.append('@asil')

            self.warnings.append({
                'file': str(file_path),
                'line': 1,
                'type': 'FILE_TRACE_MISSING',
                'message': f"文件缺少追溯注释: {', '.join(missing)}"
            })

    def _check_function_traceability(self, content, file_path):
        """检查函数级追溯注释"""
        lines = content.split('\n')

        i = 0
        while i < len(lines):
            line = lines[i]

            # 查找函数定义(仅检查头文件中的公共接口)
            if str(file_path).endswith('.h'):
                func_match = re.match(
                    r'^\s*(?:const\s+)?(?:unsigned\s+)?(?:signed\s+)?'
                    r'(?:void|int|uint8_t|uint16_t|uint32_t|int8_t|int16_t|int32_t|'
                    r'bool_t|ErrorCode|EntityId|[A-Z][a-zA-Z0-9_]*\*?)\s+'
                    r'([a-z_][a-z0-9_]*)\s*\(',
                    line
                )

                if func_match:
                    func_name = func_match.group(1)
                    line_no = i + 1
                    self.checked_functions += 1

                    # 向前查找注释中的追溯信息
                    trace_info = self._find_trace_info(lines, i)

                    if not trace_info['has_req'] or not trace_info['has_design'] or not trace_info['has_asil']:
                        missing = []
                        if not trace_info['has_req']:
                            missing.append('@req')
                        if not trace_info['has_design']:
                            missing.append('@design')
                        if not trace_info['has_asil']:
                            missing.append('@asil')

                        self.errors.append({
                            'file': str(file_path),
                            'line': line_no,
                            'type': 'FUNC_TRACE_MISSING',
                            'message': f"函数'{func_name}'缺少追溯注释: {', '.join(missing)}"
                        })

            i += 1

    def _find_trace_info(self, lines, func_line_idx):
        """向前查找追溯信息"""
        trace_info = {
            'has_req': False,
            'has_design': False,
            'has_asil': False
        }

        # 向前查找最多15行
        for i in range(max(0, func_line_idx - 15), func_line_idx):
            line = lines[i]
            if '@req:' in line or '@req ' in line:
                trace_info['has_req'] = True
            if '@design:' in line or '@design ' in line:
                trace_info['has_design'] = True
            if '@asil:' in line or '@asil ' in line:
                trace_info['has_asil'] = True

        return trace_info

    def run(self):
        """运行检查"""
        print(f"开始检查追溯信息: {self.root_dir}")
        print("=" * 70)

        # 遍历所有 .c 和 .h 文件
        for ext in ['*.c', '*.h']:
            for file_path in self.root_dir.rglob(ext):
                # 跳过 build 和 port 目录
                if 'build' in str(file_path) or 'port' in str(file_path):
                    continue
                self.check_file(file_path)

        # 输出结果
        self._print_results()

        return len(self.errors) == 0

    def _print_results(self):
        """打印检查结果"""
        print(f"\n统计:")
        print(f"  检查文件数: {self.checked_files}")
        print(f"  检查函数数: {self.checked_functions}")

        if self.errors:
            print("\n❌ 发现追溯信息错误:")
            print("-" * 70)
            for err in self.errors[:15]:  # 只显示前15个
                print(f"{err['file']}:{err['line']}")
                print(f"  {err['message']}")
            if len(self.errors) > 15:
                print(f"  ... 还有 {len(self.errors) - 15} 个错误")

        if self.warnings:
            print("\n⚠️  追溯信息警告:")
            print("-" * 70)
            for warn in self.warnings[:10]:  # 只显示前10个
                print(f"{warn['file']}:{warn['line']}")
                print(f"  {warn['message']}")
            if len(self.warnings) > 10:
                print(f"  ... 还有 {len(self.warnings) - 10} 个警告")

        print("\n" + "=" * 70)
        print(f"检查完成:")
        print(f"  错误: {len(self.errors)}")
        print(f"  警告: {len(self.warnings)}")

        if len(self.errors) == 0:
            print("✅ 所有追溯信息完整!")
        else:
            print("❌ 存在追溯信息缺失,请补充!")


def main():
    if len(sys.argv) < 2:
        print("用法: python3 check_traceability.py <代码根目录>")
        sys.exit(1)

    root_dir = sys.argv[1]
    if not os.path.exists(root_dir):
        print(f"错误: 目录不存在: {root_dir}")
        sys.exit(1)

    checker = TraceabilityChecker(root_dir)
    success = checker.run()

    sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()
