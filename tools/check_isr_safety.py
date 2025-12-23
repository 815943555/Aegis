#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ISR安全检查脚本
检查中断安全标记是否完整,以及ISR中是否调用了ISR_UNSAFE函数

作者: jack liu
"""

import os
import re
import sys
from pathlib import Path


class ISRSafetyChecker:
    """ISR安全检查器"""

    def __init__(self, root_dir):
        self.root_dir = Path(root_dir)
        self.errors = []
        self.warnings = []
        self.isr_safe_functions = set()
        self.isr_unsafe_functions = set()
        self.unmarked_functions = {}

    def check_file(self, file_path):
        """检查单个文件"""
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()

        # 第一遍:提取所有函数及其ISR标记
        self._extract_function_safety(content, file_path)

        # 第二遍:检查ISR上下文中的函数调用
        self._check_isr_calls(content, file_path)

    def _extract_function_safety(self, content, file_path):
        """提取函数及其ISR安全标记"""
        lines = content.split('\n')

        # 查找函数定义及其前面的注释
        i = 0
        while i < len(lines):
            line = lines[i]

            # 查找函数定义
            func_match = re.match(
                r'^\s*(?:static\s+)?(?:inline\s+)?'
                r'(?:const\s+)?(?:unsigned\s+)?(?:signed\s+)?'
                r'(?:void|int|uint8_t|uint16_t|uint32_t|int8_t|int16_t|int32_t|'
                r'bool_t|ErrorCode|EntityId|[A-Z][a-zA-Z0-9_]*\*?)\s+'
                r'([a-z_][a-z0-9_]*)\s*\(',
                line
            )

            if func_match:
                func_name = func_match.group(1)
                line_no = i + 1

                # 向前查找注释中的ISR标记
                isr_mark = self._find_isr_mark(lines, i)

                if isr_mark == 'isr_safe':
                    self.isr_safe_functions.add(func_name)
                elif isr_mark == 'isr_unsafe':
                    self.isr_unsafe_functions.add(func_name)
                else:
                    # 未标记的函数
                    self.unmarked_functions[func_name] = (str(file_path), line_no)

            i += 1

    def _find_isr_mark(self, lines, func_line_idx):
        """向前查找ISR标记"""
        # 向前查找最多10行
        for i in range(max(0, func_line_idx - 10), func_line_idx):
            line = lines[i]
            if '@isr_safe' in line or 'ISR_SAFE' in line:
                return 'isr_safe'
            if '@isr_unsafe' in line or 'ISR_UNSAFE' in line:
                return 'isr_unsafe'
        return None

    def _check_isr_calls(self, content, file_path):
        """检查ISR中的函数调用"""
        lines = content.split('\n')

        # 查找可能的ISR函数(函数名包含isr/irq/handler)
        in_isr_function = False
        current_isr_name = None

        for i, line in enumerate(lines, 1):
            # 检测ISR函数开始
            isr_func_match = re.match(
                r'^\s*(?:void|int)\s+([a-z_]*(?:isr|irq|handler|interrupt)[a-z0-9_]*)\s*\(',
                line, re.IGNORECASE
            )
            if isr_func_match:
                in_isr_function = True
                current_isr_name = isr_func_match.group(1)

            # 检测函数结束
            if in_isr_function and re.match(r'^\}', line):
                in_isr_function = False
                current_isr_name = None

            # 在ISR中检查函数调用
            if in_isr_function:
                # 提取函数调用: func_name(...)
                call_matches = re.findall(r'([a-z_][a-z0-9_]*)\s*\(', line)
                for called_func in call_matches:
                    if called_func in self.isr_unsafe_functions:
                        self.errors.append({
                            'file': str(file_path),
                            'line': i,
                            'type': 'ISR_UNSAFE_CALL',
                            'message': f"ISR函数'{current_isr_name}'调用了ISR_UNSAFE函数'{called_func}'"
                        })

    def run(self):
        """运行检查"""
        print(f"开始检查ISR安全: {self.root_dir}")
        print("=" * 70)

        # 第一遍:扫描所有文件,收集函数标记
        for ext in ['*.c', '*.h']:
            for file_path in self.root_dir.rglob(ext):
                if 'build' in str(file_path):
                    continue
                self.check_file(file_path)

        # 检查未标记的函数
        for func_name, (file_path, line_no) in self.unmarked_functions.items():
            # 跳过内部辅助函数(static)和明显的私有函数
            if func_name.startswith('_') or 'test' in file_path:
                continue

            self.warnings.append({
                'file': file_path,
                'line': line_no,
                'type': 'ISR_UNMARKED',
                'message': f"函数'{func_name}'缺少ISR安全标记(@isr_safe 或 @isr_unsafe)"
            })

        # 输出结果
        self._print_results()

        return len(self.errors) == 0

    def _print_results(self):
        """打印检查结果"""
        print(f"\n统计:")
        print(f"  ISR_SAFE函数: {len(self.isr_safe_functions)}")
        print(f"  ISR_UNSAFE函数: {len(self.isr_unsafe_functions)}")
        print(f"  未标记函数: {len(self.unmarked_functions)}")

        if self.errors:
            print("\n❌ 发现ISR安全错误:")
            print("-" * 70)
            for err in self.errors:
                print(f"{err['file']}:{err['line']}")
                print(f"  {err['message']}")

        if self.warnings:
            print("\n⚠️  ISR安全警告:")
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
            print("✅ 所有ISR调用安全!")
        else:
            print("❌ 存在ISR安全错误,请修正!")


def main():
    if len(sys.argv) < 2:
        print("用法: python3 check_isr_safety.py <代码根目录>")
        sys.exit(1)

    root_dir = sys.argv[1]
    if not os.path.exists(root_dir):
        print(f"错误: 目录不存在: {root_dir}")
        sys.exit(1)

    checker = ISRSafetyChecker(root_dir)
    success = checker.run()

    sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()
