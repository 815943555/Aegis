/*
 * @file: mem_pool.h
 * @brief: 统一内存块静态内存池接口（支持多种块大小）
 * @author: jack liu
 * @req: REQ-MEM-001
 * @design: DES-MEM-001
 * @asil: ASIL-B
 */

#ifndef MEM_POOL_H
#define MEM_POOL_H

#include "types.h"
#include "error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 内存池配置 ==================== */
/* 通过 CMake 传入，提供默认值 */
/* 单一内存块，按区域划分为：小块、中块、大块、超大块 */

/* 小块配置 */
#ifndef MEM_POOL_SMALL_SIZE
#define MEM_POOL_SMALL_SIZE     32      /* 小块大小（字节） */
#endif
#ifndef MEM_POOL_SMALL_COUNT
#define MEM_POOL_SMALL_COUNT    16      /* 小块数量 */
#endif

/* 中块配置 */
#ifndef MEM_POOL_MEDIUM_SIZE
#define MEM_POOL_MEDIUM_SIZE    64      /* 中块大小（字节） */
#endif
#ifndef MEM_POOL_MEDIUM_COUNT
#define MEM_POOL_MEDIUM_COUNT   8       /* 中块数量 */
#endif

/* 大块配置 */
#ifndef MEM_POOL_LARGE_SIZE
#define MEM_POOL_LARGE_SIZE     128     /* 大块大小（字节） */
#endif
#ifndef MEM_POOL_LARGE_COUNT
#define MEM_POOL_LARGE_COUNT    4       /* 大块数量 */
#endif

/* 超大块配置 */
#ifndef MEM_POOL_XLARGE_SIZE
#define MEM_POOL_XLARGE_SIZE    256     /* 超大块大小（字节） */
#endif
#ifndef MEM_POOL_XLARGE_COUNT
#define MEM_POOL_XLARGE_COUNT   2       /* 超大块数量 */
#endif

/* 计算总内存池大小（编译期计算） */
#define MEM_POOL_TOTAL_SIZE \
    ((MEM_POOL_SMALL_SIZE * MEM_POOL_SMALL_COUNT) + \
     (MEM_POOL_MEDIUM_SIZE * MEM_POOL_MEDIUM_COUNT) + \
     (MEM_POOL_LARGE_SIZE * MEM_POOL_LARGE_COUNT) + \
     (MEM_POOL_XLARGE_SIZE * MEM_POOL_XLARGE_COUNT))

#define MEM_POOL_TOTAL_BLOCKS \
    (MEM_POOL_SMALL_COUNT + MEM_POOL_MEDIUM_COUNT + \
     MEM_POOL_LARGE_COUNT + MEM_POOL_XLARGE_COUNT)

/* ==================== 内存池统计信息 ==================== */
typedef struct {
    uint8_t total_blocks;       /* 总块数 */
    uint8_t used_blocks;        /* 已用块数 */
    uint8_t free_blocks;        /* 空闲块数 */
    uint8_t peak_usage;         /* 峰值使用量 */

    /* 各类型块统计 */
    uint8_t small_used;         /* 小块已用 */
    uint8_t medium_used;        /* 中块已用 */
    uint8_t large_used;         /* 大块已用 */
    uint8_t xlarge_used;        /* 超大块已用 */
} AegisMemPoolStats;

/* ==================== 内存池接口 ==================== */
/*
 * @brief: 初始化内存池（幂等操作）
 * @return: 错误码
 * @req: REQ-MEM-002
 * @design: DES-MEM-002
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_mem_pool_init(void);

/*
 * @brief: 分配内存块（自动选择最合适的块大小）
 * @param size: 请求大小（字节）
 * @param file: 分配发起文件名（用于追溯）
 * @param line: 分配发起行号（用于追溯）
 * @return: 分配的内存指针，失败返回 NULL
 * @req: REQ-MEM-003
 * @design: DES-MEM-003
 * @asil: ASIL-B
 * @isr_unsafe
 */
void* aegis_mem_pool_alloc(uint32_t size, const char* file, uint32_t line);

/*
 * @brief: 释放内存块
 * @param ptr: 待释放指针
 * @return: 错误码
 * @req: REQ-MEM-004
 * @design: DES-MEM-004
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_mem_pool_free(void* ptr);

/*
 * @brief: 获取内存池使用统计
 * @param stats: 输出统计信息
 * @return: 错误码
 * @req: REQ-MEM-005
 * @design: DES-MEM-005
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_mem_pool_get_stats(AegisMemPoolStats* stats);

/*
 * @brief: 获取内存池基地址（用于DMA等场景）
 * @return: 内存池基地址
 * @req: REQ-MEM-006
 * @design: DES-MEM-006
 * @asil: ASIL-B
 * @isr_safe
 */
const void* aegis_mem_pool_get_base_addr(void);

/*
 * @brief: 获取内存池总大小
 * @return: 内存池总大小（字节）
 * @req: REQ-MEM-007
 * @design: DES-MEM-007
 * @asil: ASIL-B
 * @isr_safe
 */
uint32_t aegis_mem_pool_get_total_size(void);

/*
 * @brief: 检查指定内存块的魔法数（检测溢出）
 * @param ptr: 内存块指针
 * @return: 错误码，ERR_OK表示魔法数完整
 * @req: REQ-MEM-008
 * @design: DES-MEM-008
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_mem_pool_check_magic(const void* ptr);

/*
 * @brief: 检查所有已分配块的魔法数
 * @param corrupted_count: 输出损坏的块数量
 * @return: 错误码，ERR_OK表示所有块完整
 * @req: REQ-MEM-009
 * @design: DES-MEM-009
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_mem_pool_check_all_magic(uint8_t* corrupted_count);

/* ==================== 便捷分配宏 ==================== */
/* 自动记录文件名和行号 */
#define MEM_ALLOC(size)     aegis_mem_pool_alloc((size), __FILE__, __LINE__)
#define MEM_FREE(ptr)       aegis_mem_pool_free(ptr)

#ifdef __cplusplus
}
#endif

#endif /* MEM_POOL_H */
