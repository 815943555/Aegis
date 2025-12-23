#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
命名规范检查脚本
检查C代码是否符合DDD四层架构的命名前缀约束

作者: jack liu
"""

import os
import re
import sys
from pathlib import Path


class NamingChecker:
    """命名规范检查器"""

    # 层级前缀规则
    LAYER_RULES = {
        'infrastructure': {
            'function_prefix': ['aegis_infrastructure_', 'aegis_hal_'],
            'type_prefix': ['AegisInfrastructure', 'AegisHal', 'HAL_'],
            'paths': ['include/infrastructure', 'src/infrastructure']
        },
        'hal': {
            'function_prefix': ['aegis_hal_', 'aegis_entry_platform_'],
            'type_prefix': ['AegisHal', 'HAL_'],
            'paths': ['include/hal', 'src/hal', 'port']
        },
        'domain': {
            'function_prefix': ['aegis_domain_'],
            'type_prefix': ['AegisDomain', 'AegisEntity', 'AegisEvent'],
            'paths': ['include/domain', 'src/domain']
        },
        'application': {
            'function_prefix': ['aegis_app_cmd_', 'aegis_app_query_', 'aegis_app_init_', 'aegis_app_asm_', 'aegis_app_conv_', 'aegis_app_dto_', 'aegis_app_'],
            'type_prefix': ['AegisAppCmd', 'AegisAppQuery', 'AegisCommand', 'AegisQuery', 'AegisApp', 'AegisDto'],
            'paths': ['include/application', 'src/application']
        },
        'entry': {
            'function_prefix': ['aegis_entry_'],
            'type_prefix': ['AegisEntry'],
            'paths': ['include/entry', 'src/entry']
        },
        'common': {
            'function_prefix': ['aegis_mem_pool_', 'aegis_ring_buffer_', 'aegis_trace_', 'aegis_error_code_', 'aegis_critical_'],
            'type_prefix': ['AegisMemPool', 'AegisRingBuffer', 'AegisTrace', 'AegisErrorCode', 'AegisError'],
            'paths': ['include/common', 'src/common']
        }
    }

    # 事件命名规则
    EVENT_TYPE_PATTERN = re.compile(r'^\s*DOMAIN_EVENT_[A-Z_]+\s*=')
    EVENT_HANDLER_PATTERN = re.compile(r'^\s*(?:static\s+)?EventHandlerResult\s+([a-z_][a-z0-9_]*)\s*\(')

    def __init__(self, root_dir):
        self.root_dir = Path(root_dir)
        self.errors = []
        self.warnings = []

    def check_file(self, file_path):
        """检查单个文件"""
        # 判断文件所属层级
        layer = self._detect_layer(file_path)
        if not layer:
            return  # 跳过无法识别的文件

        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()

        # 提取函数定义
        functions = self._extract_functions(content)
        for func_name, line_no, is_static in functions:
            self._check_function_name(func_name, line_no, layer, file_path, is_static)

        # 提取类型定义
        types = self._extract_types(content)
        for type_name, line_no in types:
            self._check_type_name(type_name, line_no, layer, file_path)

        # 检查事件类型命名
        event_types = self._extract_event_types(content)
        for event_type, line_no in event_types:
            self._check_event_type_name(event_type, line_no, layer, file_path)

        # 检查事件处理器命名
        event_handlers = self._extract_event_handlers(content)
        for handler_name, line_no in event_handlers:
            self._check_event_handler_name(handler_name, line_no, layer, file_path)

    def _detect_layer(self, file_path):
        """检测文件所属层级"""
        file_path_str = str(file_path)

        # 特殊文件名优先检测（port目录下的critical文件属于common层）
        if 'critical' in file_path_str and 'port' in file_path_str:
            return 'common'

        for layer, rules in self.LAYER_RULES.items():
            for path_pattern in rules['paths']:
                if path_pattern in file_path_str:
                    return layer
        return None

    def _extract_functions(self, content):
        """提取函数定义 (函数名, 行号, 是否静态)"""
        functions = []
        lines = content.split('\n')

        # 匹配函数定义: 返回类型 函数名(参数)
        func_pattern = re.compile(
            r'^\s*(static\s+)?(?:inline\s+)?'
            r'(?:const\s+)?(?:unsigned\s+)?(?:signed\s+)?'
            r'(?:void|int|uint8_t|uint16_t|uint32_t|int8_t|int16_t|int32_t|'
            r'bool_t|ErrorCode|EntityId|[A-Z][a-zA-Z0-9_]*\*?)\s+'
            r'([a-z_][a-z0-9_]*)\s*\('
        )

        for i, line in enumerate(lines, 1):
            # 跳过注释行
            if line.strip().startswith('/*') or line.strip().startswith('*'):
                continue
            match = func_pattern.match(line)
            if match:
                is_static = match.group(1) is not None  # 检测是否有 static 关键字
                func_name = match.group(2)
                functions.append((func_name, i, is_static))

        return functions

    def _extract_types(self, content):
        """提取类型定义 (类型名, 行号)"""
        types = []
        lines = content.split('\n')

        # 匹配 typedef struct/enum
        type_pattern = re.compile(
            r'^\s*typedef\s+(?:struct|enum)\s+(?:\{[^}]*\}\s*)?([A-Z][a-zA-Z0-9_]*)\s*;'
        )

        # 匹配 typedef XXX YYY;
        simple_typedef = re.compile(
            r'^\s*typedef\s+[a-z_][a-z0-9_]*\s+([A-Z][a-zA-Z0-9_]*)\s*;'
        )

        for i, line in enumerate(lines, 1):
            match = type_pattern.search(line)
            if not match:
                match = simple_typedef.search(line)
            if match:
                type_name = match.group(1)
                types.append((type_name, i))

        return types

    def _check_function_name(self, func_name, line_no, layer, file_path, is_static=False):
        """检查函数命名是否符合层级前缀规则"""
        # 静态内部函数不强制检查层级前缀（实现细节，作用域限制）
        if is_static:
            return

        rules = self.LAYER_RULES[layer]

        # 检查是否有正确的前缀
        has_valid_prefix = any(
            func_name.startswith(prefix)
            for prefix in rules['function_prefix']
        )

        if not has_valid_prefix:
            self.errors.append({
                'file': str(file_path),
                'line': line_no,
                'type': 'NAMING_ERROR',
                'message': f"函数 '{func_name}' 缺少 {layer} 层前缀 {rules['function_prefix']}"
            })

    def _check_type_name(self, type_name, line_no, layer, file_path):
        """检查类型命名是否符合层级前缀规则"""
        rules = self.LAYER_RULES[layer]

        # 检查是否有正确的前缀
        has_valid_prefix = any(
            type_name.startswith(prefix)
            for prefix in rules['type_prefix']
        )

        if not has_valid_prefix:
            self.warnings.append({
                'file': str(file_path),
                'line': line_no,
                'type': 'NAMING_WARNING',
                'message': f"类型 '{type_name}' 建议使用 {layer} 层前缀 {rules['type_prefix']}"
            })

    def _extract_event_types(self, content):
        """提取事件类型枚举 (事件类型名, 行号)"""
        event_types = []
        lines = content.split('\n')

        for i, line in enumerate(lines, 1):
            # 跳过注释行
            if line.strip().startswith('/*') or line.strip().startswith('*'):
                continue
            match = self.EVENT_TYPE_PATTERN.match(line)
            if match:
                # 提取事件类型名称
                event_type = line.strip().split('=')[0].strip().rstrip(',')
                event_types.append((event_type, i))

        return event_types

    def _extract_event_handlers(self, content):
        """提取事件处理器函数 (处理器名, 行号)"""
        handlers = []
        lines = content.split('\n')

        for i, line in enumerate(lines, 1):
            # 跳过注释行
            if line.strip().startswith('/*') or line.strip().startswith('*'):
                continue
            match = self.EVENT_HANDLER_PATTERN.match(line)
            if match:
                handler_name = match.group(1)
                handlers.append((handler_name, i))

        return handlers

    def _check_event_type_name(self, event_type, line_no, layer, file_path):
        """检查事件类型命名是否符合规范"""
        # 事件类型只能在领域层定义
        if layer != 'domain':
            self.errors.append({
                'file': str(file_path),
                'line': line_no,
                'type': 'EVENT_NAMING_ERROR',
                'message': f"事件类型 '{event_type}' 只能在领域层定义"
            })
            return

        # 检查是否有 DOMAIN_EVENT_ 前缀
        if not event_type.startswith('DOMAIN_EVENT_'):
            self.errors.append({
                'file': str(file_path),
                'line': line_no,
                'type': 'EVENT_NAMING_ERROR',
                'message': f"事件类型 '{event_type}' 必须以 'DOMAIN_EVENT_' 开头"
            })

    def _check_event_handler_name(self, handler_name, line_no, layer, file_path):
        """检查事件处理器命名是否符合规范"""
        # 事件处理器建议使用 on_ 前缀
        if not handler_name.startswith('on_'):
            self.warnings.append({
                'file': str(file_path),
                'line': line_no,
                'type': 'EVENT_HANDLER_WARNING',
                'message': f"事件处理器 '{handler_name}' 建议以 'on_' 开头 (如 on_sensor_created)"
            })

    def run(self):
        """运行检查"""
        print(f"开始检查命名规范: {self.root_dir}")
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
            print("\n❌ 发现命名错误:")
            print("-" * 70)
            for err in self.errors:
                print(f"{err['file']}:{err['line']}")
                print(f"  {err['message']}")

        if self.warnings:
            print("\n⚠️  命名警告:")
            print("-" * 70)
            for warn in self.warnings:
                print(f"{warn['file']}:{warn['line']}")
                print(f"  {warn['message']}")

        print("\n" + "=" * 70)
        print(f"检查完成:")
        print(f"  错误: {len(self.errors)}")
        print(f"  警告: {len(self.warnings)}")

        # 统计事件相关的检查
        event_errors = sum(1 for e in self.errors if 'EVENT' in e['type'])
        event_warnings = sum(1 for w in self.warnings if 'EVENT' in w['type'])
        if event_errors > 0 or event_warnings > 0:
            print(f"\n事件相关检查:")
            print(f"  事件错误: {event_errors}")
            print(f"  事件警告: {event_warnings}")

        if len(self.errors) == 0:
            print("✅ 所有函数命名符合规范!")
        else:
            print("❌ 存在命名错误,请修正!")


def main():
    if len(sys.argv) < 2:
        print("用法: python3 check_naming.py <代码根目录>")
        sys.exit(1)

    root_dir = sys.argv[1]
    if not os.path.exists(root_dir):
        print(f"错误: 目录不存在: {root_dir}")
        sys.exit(1)

    checker = NamingChecker(root_dir)
    success = checker.run()

    sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()
