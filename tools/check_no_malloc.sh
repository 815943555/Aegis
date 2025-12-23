#!/bin/bash
# 禁止动态内存分配检查脚本
# 检查代码中是否使用了malloc/calloc/realloc/free
#
# 作者: jack liu

set -e

if [ $# -lt 1 ]; then
    echo "用法: $0 <代码根目录>"
    exit 1
fi

ROOT_DIR="$1"

if [ ! -d "$ROOT_DIR" ]; then
    echo "错误: 目录不存在: $ROOT_DIR"
    exit 1
fi

echo "开始检查动态内存分配: $ROOT_DIR"
echo "======================================================================"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 查找所有 .c 和 .h 文件
FOUND_ERRORS=0

# 检查 malloc
echo -e "\n检查 malloc() 调用..."
MALLOC_RESULTS=$(find "$ROOT_DIR" -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/build/*" -exec grep -Hn '\bmalloc\s*(' {} \; || true)
if [ -n "$MALLOC_RESULTS" ]; then
    echo -e "${RED}❌ 发现 malloc() 调用:${NC}"
    echo "$MALLOC_RESULTS"
    FOUND_ERRORS=$((FOUND_ERRORS + 1))
fi

# 检查 calloc
echo -e "\n检查 calloc() 调用..."
CALLOC_RESULTS=$(find "$ROOT_DIR" -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/build/*" -exec grep -Hn '\bcalloc\s*(' {} \; || true)
if [ -n "$CALLOC_RESULTS" ]; then
    echo -e "${RED}❌ 发现 calloc() 调用:${NC}"
    echo "$CALLOC_RESULTS"
    FOUND_ERRORS=$((FOUND_ERRORS + 1))
fi

# 检查 realloc
echo -e "\n检查 realloc() 调用..."
REALLOC_RESULTS=$(find "$ROOT_DIR" -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/build/*" -exec grep -Hn '\brealloc\s*(' {} \; || true)
if [ -n "$REALLOC_RESULTS" ]; then
    echo -e "${RED}❌ 发现 realloc() 调用:${NC}"
    echo "$REALLOC_RESULTS"
    FOUND_ERRORS=$((FOUND_ERRORS + 1))
fi

# 检查 free (排除 mem_pool_free 和 ring_buffer_get_free)
echo -e "\n检查 free() 调用..."
FREE_RESULTS=$(find "$ROOT_DIR" -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/build/*" -exec grep -Hn '\bfree\s*(' {} \; | grep -v 'mem_pool_free' | grep -v 'ring_buffer_get_free' | grep -v '_free(' || true)
if [ -n "$FREE_RESULTS" ]; then
    echo -e "${RED}❌ 发现 free() 调用:${NC}"
    echo "$FREE_RESULTS"
    FOUND_ERRORS=$((FOUND_ERRORS + 1))
fi

# 检查 stdlib.h 包含
echo -e "\n检查 <stdlib.h> 包含..."
STDLIB_RESULTS=$(find "$ROOT_DIR" -type f \( -name "*.c" -o -name "*.h" \) ! -path "*/build/*" -exec grep -Hn '#include\s*<stdlib.h>' {} \; || true)
if [ -n "$STDLIB_RESULTS" ]; then
    echo -e "${YELLOW}⚠️  发现 stdlib.h 包含(可能使用动态内存):${NC}"
    echo "$STDLIB_RESULTS"
fi

echo -e "\n======================================================================"
if [ $FOUND_ERRORS -eq 0 ]; then
    echo -e "${GREEN}✅ 检查通过: 未发现动态内存分配!${NC}"
    exit 0
else
    echo -e "${RED}❌ 检查失败: 发现 $FOUND_ERRORS 处动态内存分配违规!${NC}"
    echo -e "${YELLOW}提示: 请使用 mem_pool_alloc()/mem_pool_free() 替代${NC}"
    exit 1
fi
