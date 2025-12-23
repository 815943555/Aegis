#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
分层依赖检查脚本
检查四层架构的依赖关系是否符合约束

作者: jack liu
"""

import os
import re
import sys
from pathlib import Path


class LayeringChecker:
    """分层依赖检查器"""

    # 层级依赖规则: 层级 -> 允许依赖的层级
    LAYER_DEPENDENCY_RULES = {
        # 严格DDD依赖：外层依赖内层（infrastructure/application/entry -> domain -> common）
        'infrastructure': ['domain', 'common'],         # Infrastructure 可以依赖 Domain 接口与公共层
        'domain': ['common'],                            # Domain 不依赖 Infrastructure
        'application': ['domain', 'common'],             # Application 不依赖 Infrastructure（由 Entry 组合注入）
        'entry': ['application', 'domain', 'common'],  # Entry 作为组合根，不直接依赖 Infrastructure
        'common': []                         # 公共层不依赖其他层
    }

    # 层级路径映射
    LAYER_PATHS = {
        'infrastructure': ['include/infrastructure', 'src/infrastructure', 'port'],
        'domain': ['include/domain', 'src/domain'],
        'application': ['include/application', 'src/application'],
        'entry': ['include/entry', 'src/entry'],
        'common': ['include/common', 'src/common']
    }

    # 层级头文件前缀
    LAYER_HEADERS = {
        'infrastructure': ['infrastructure_', 'hal_'],  # 保留hal_以兼容旧文件
        'domain': ['domain_'],
        'application': ['app_'],
        'entry': ['entry_'],
        'common': ['types.h', 'error_codes.h', 'critical.h', 'mem_pool.h',
                   'ring_buffer.h', 'trace.h', 'isr_safety.h']
    }

    # 事件发布函数（只能在领域层调用）
    EVENT_PUBLISH_PATTERN = re.compile(r'\baegis_domain_event_publish\s*\(')

    def __init__(self, root_dir):
        self.root_dir = Path(root_dir)
        self.errors = []
        self.warnings = []

    def check_file(self, file_path):
        """检查单个文件的依赖"""
        # 判断文件所属层级
        layer = self._detect_layer(file_path)
        if not layer:
            return

        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()

        # 提取 #include 语句
        includes = self._extract_includes(content)

        for include_file, line_no in includes:
            self._check_include(include_file, line_no, layer, file_path)

        # 检查事件发布调用
        event_publishes = self._extract_event_publishes(content)
        for line_no in event_publishes:
            self._check_event_publish(line_no, layer, file_path)

    def _detect_layer(self, file_path):
        """检测文件所属层级"""
        file_path_str = str(file_path)
        for layer, paths in self.LAYER_PATHS.items():
            for path_pattern in paths:
                if path_pattern in file_path_str:
                    return layer
        return None

    def _extract_includes(self, content):
        """提取 #include 语句 (文件名, 行号)"""
        includes = []
        lines = content.split('\n')

        # 匹配 #include "xxx.h"
        include_pattern = re.compile(r'^\s*#include\s+"([^"]+)"')

        for i, line in enumerate(lines, 1):
            match = include_pattern.match(line)
            if match:
                include_file = match.group(1)
                includes.append((include_file, i))

        return includes

    def _detect_include_layer(self, include_file):
        """检测被包含文件所属层级"""
        for layer, prefixes in self.LAYER_HEADERS.items():
            for prefix in prefixes:
                if include_file.startswith(prefix) or include_file == prefix:
                    return layer
        return None

    def _check_include(self, include_file, line_no, current_layer, file_path):
        """检查包含关系是否合法"""
        included_layer = self._detect_include_layer(include_file)

        if not included_layer:
            # 无法识别的头文件(如标准库),跳过
            return

        if included_layer == current_layer:
            # 同层依赖,允许
            return

        # 检查是否在允许的依赖列表中
        allowed_layers = self.LAYER_DEPENDENCY_RULES.get(current_layer, [])

        if included_layer not in allowed_layers:
            self.errors.append({
                'file': str(file_path),
                'line': line_no,
                'type': 'LAYER_VIOLATION',
                'message': f"{current_layer}层不允许依赖{included_layer}层 (包含'{include_file}')"
            })

    def _extract_event_publishes(self, content):
        """提取事件发布调用 (行号列表)"""
        publishes = []
        lines = content.split('\n')

        for i, line in enumerate(lines, 1):
            # 跳过注释行
            if line.strip().startswith('/*') or line.strip().startswith('*'):
                continue
            if self.EVENT_PUBLISH_PATTERN.search(line):
                publishes.append(i)

        return publishes

    def _check_event_publish(self, line_no, layer, file_path):
        """检查事件发布是否在正确的层级"""
        # 事件发布只能在领域层调用
        if layer != 'domain':
            self.errors.append({
                'file': str(file_path),
                'line': line_no,
                'type': 'EVENT_PUBLISH_VIOLATION',
                'message': f"domain_event_publish() 只能在领域层调用,当前层级: {layer}"
            })

    def run(self):
        """运行检查"""
        print(f"开始检查分层依赖: {self.root_dir}")
        print("=" * 70)

        # 遍历所有 .c 和 .h 文件
        for ext in ['*.c', '*.h']:
            for file_path in self.root_dir.rglob(ext):
                # 跳过 build 目录
                if 'build' in str(file_path):
                    continue
                self.check_file(file_path)

        # 输出结果
        self._print_results()

        return len(self.errors) == 0

    def _print_results(self):
        """打印检查结果"""
        if self.errors:
            print("\n❌ 发现分层依赖违规:")
            print("-" * 70)
            for err in self.errors:
                print(f"{err['file']}:{err['line']}")
                print(f"  {err['message']}")

        if self.warnings:
            print("\n⚠️  分层依赖警告:")
            print("-" * 70)
            for warn in self.warnings:
                print(f"{warn['file']}:{warn['line']}")
                print(f"  {warn['message']}")

        print("\n" + "=" * 70)
        print(f"检查完成:")
        print(f"  错误: {len(self.errors)}")
        print(f"  警告: {len(self.warnings)}")

        # 统计事件发布相关的检查
        event_errors = sum(1 for e in self.errors if 'EVENT_PUBLISH' in e['type'])
        if event_errors > 0:
            print(f"\n事件发布检查:")
            print(f"  事件发布违规: {event_errors}")

        if len(self.errors) == 0:
            print("✅ 所有分层依赖符合约束!")
        else:
            print("❌ 存在分层依赖违规,请修正!")


def main():
    if len(sys.argv) < 2:
        print("用法: python3 check_layering.py <代码根目录>")
        sys.exit(1)

    root_dir = sys.argv[1]
    if not os.path.exists(root_dir):
        print(f"错误: 目录不存在: {root_dir}")
        sys.exit(1)

    checker = LayeringChecker(root_dir)
    success = checker.run()

    sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()
