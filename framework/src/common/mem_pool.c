/*
 * @file: mem_pool.c
 * @brief: 统一内存块静态内存池实现（单一内存块+多区域划分+魔法数保护）
 * @author: jack liu
 */

#include "mem_pool.h"
#include "critical.h"
#include <string.h>

/* ==================== 魔法数定义 ==================== */
#define MEM_MAGIC_HEAD      0xDEADU         /* 头部魔法数（16位） */
#define MEM_MAGIC_TAIL      0xBEEFU         /* 尾部魔法数（16位） */
#define MEM_MAGIC_SIZE      2               /* 魔法数占用字节 */

/* 空闲链表结束标记 */
#define FREE_LIST_END   0xFF

/* ==================== 内部辅助函数 ==================== */
/*
 * @brief: 写入魔法数到内存块头尾
 */
static void write_magic_numbers(uint8_t* block_start, uint16_t block_size) {
    uint16_t magic;

    /* 写入头部魔法数 */
    magic = MEM_MAGIC_HEAD;
    memcpy(block_start, &magic, MEM_MAGIC_SIZE);

    /* 写入尾部魔法数 */
    magic = MEM_MAGIC_TAIL;
    memcpy(block_start + block_size - MEM_MAGIC_SIZE, &magic, MEM_MAGIC_SIZE);
}

/*
 * @brief: 检查魔法数是否完整
 * @return: TRUE=完整，FALSE=损坏
 */
static bool_t check_magic_numbers(const uint8_t* block_start, uint16_t block_size) {
    uint16_t magic_head;
    uint16_t magic_tail;

    /* 检查头部魔法数 */
    memcpy(&magic_head, block_start, MEM_MAGIC_SIZE);
    if (magic_head != MEM_MAGIC_HEAD) {
        return FALSE;
    }

    /* 检查尾部魔法数 */
    memcpy(&magic_tail, block_start + block_size - MEM_MAGIC_SIZE, MEM_MAGIC_SIZE);
    if (magic_tail != MEM_MAGIC_TAIL) {
        return FALSE;
    }

    return TRUE;
}

/*
 * @brief: 获取指针所属的区域和块索引
 * @return: 区域索引，-1表示无效指针
 */
static int8_t find_block_region(const AegisMemPool* pool, const void* ptr, uint8_t* block_idx) {
    uint8_t r;
    const uint8_t* p = (const uint8_t*)ptr;

    if (pool == NULL) {
        return -1;
    }

    for (r = 0; r < 4; r++) {
        const uint8_t* region_start = pool->regions[r].start_addr;
        uint32_t region_size = (uint32_t)pool->regions[r].block_size *
                               (uint32_t)pool->regions[r].block_count;
        const uint8_t* region_end = region_start + region_size;

        if (p >= region_start && p < region_end) {
            /* 计算块索引 */
            uint32_t offset = (uint32_t)(p - region_start);
            *block_idx = (uint8_t)(offset / (uint32_t)pool->regions[r].block_size);
            return (int8_t)r;
        }
    }

    return -1;  /* 无效指针 */
}

/*
 * @brief: 获取元数据的全局索引
 */
static uint8_t get_meta_index(const AegisMemPool* pool, uint8_t region, uint8_t block_idx) {
    uint8_t meta_idx = 0;
    uint8_t r;

    if (pool == NULL) {
        return 0;
    }

    /* 累加前面区域的块数 */
    for (r = 0; r < region; r++) {
        meta_idx += pool->regions[r].block_count;
    }

    meta_idx += block_idx;
    return meta_idx;
}

/*
 * @brief: 初始化区域的空闲链表
 */
static void init_free_list(AegisMemPool* pool, uint8_t region) {
    uint8_t i;
    uint8_t* block_addr;

    if (pool == NULL) {
        return;
    }

    /* 构建空闲链表：block[0]->block[1]->...->0xFF */
    for (i = 0; i < pool->regions[region].block_count; i++) {
        block_addr = pool->regions[region].start_addr +
                     (uint32_t)i * (uint32_t)pool->regions[region].block_size;

        if (i < pool->regions[region].block_count - 1) {
            /* 前2字节存储下一个块的索引 */
            block_addr[0] = (uint8_t)(i + 1);
        } else {
            /* 最后一个块指向结束标记 */
            block_addr[0] = FREE_LIST_END;
        }
    }

    /* 链表头指向第0块 */
    pool->regions[region].free_list_head = 0;
}

/*
 * @brief: 从空闲链表分配一块
 * @return: 块索引，FREE_LIST_END表示无空闲块
 */
static uint8_t alloc_from_free_list(AegisMemPool* pool, uint8_t region) {
    uint8_t block_idx;
    uint8_t* block_addr;

    if (pool == NULL) {
        return FREE_LIST_END;
    }

    block_idx = pool->regions[region].free_list_head;

    if (block_idx != FREE_LIST_END) {
        /* 取出链表头 */
        block_addr = pool->regions[region].start_addr +
                     (uint32_t)block_idx * (uint32_t)pool->regions[region].block_size;

        /* 更新链表头为下一个空闲块 */
        pool->regions[region].free_list_head = block_addr[0];
    }

    return block_idx;
}

/*
 * @brief: 将块归还到空闲链表
 */
static void free_to_list(AegisMemPool* pool, uint8_t region, uint8_t block_idx) {
    uint8_t* block_addr;

    if (pool == NULL) {
        return;
    }

    block_addr = pool->regions[region].start_addr +
                 (uint32_t)block_idx * (uint32_t)pool->regions[region].block_size;

    /* 插入链表头 */
    block_addr[0] = pool->regions[region].free_list_head;
    pool->regions[region].free_list_head = block_idx;
}

/* ==================== 公共接口实现 ==================== */
AegisErrorCode aegis_mem_pool_init(AegisMemPool* pool, AegisTraceLog* trace) {
    uint8_t i;
    uint8_t* current_addr;

    if (pool == NULL) {
        return ERR_NULL_PTR;
    }

    ENTER_CRITICAL();

    /* 初始化区域描述表 */
    memset(pool, 0, sizeof(AegisMemPool));
    pool->trace = trace;
    current_addr = pool->buffer;

    /* 区域0：小块 */
    pool->regions[0].start_addr = current_addr;
    pool->regions[0].block_size = MEM_POOL_SMALL_SIZE;
    pool->regions[0].block_count = MEM_POOL_SMALL_COUNT;
    pool->regions[0].used_count = 0;
    pool->regions[0].peak_usage = 0;
    current_addr += (uint32_t)MEM_POOL_SMALL_SIZE * (uint32_t)MEM_POOL_SMALL_COUNT;

    /* 区域1：中块 */
    pool->regions[1].start_addr = current_addr;
    pool->regions[1].block_size = MEM_POOL_MEDIUM_SIZE;
    pool->regions[1].block_count = MEM_POOL_MEDIUM_COUNT;
    pool->regions[1].used_count = 0;
    pool->regions[1].peak_usage = 0;
    current_addr += (uint32_t)MEM_POOL_MEDIUM_SIZE * (uint32_t)MEM_POOL_MEDIUM_COUNT;

    /* 区域2：大块 */
    pool->regions[2].start_addr = current_addr;
    pool->regions[2].block_size = MEM_POOL_LARGE_SIZE;
    pool->regions[2].block_count = MEM_POOL_LARGE_COUNT;
    pool->regions[2].used_count = 0;
    pool->regions[2].peak_usage = 0;
    current_addr += (uint32_t)MEM_POOL_LARGE_SIZE * (uint32_t)MEM_POOL_LARGE_COUNT;

    /* 区域3：超大块 */
    pool->regions[3].start_addr = current_addr;
    pool->regions[3].block_size = MEM_POOL_XLARGE_SIZE;
    pool->regions[3].block_count = MEM_POOL_XLARGE_COUNT;
    pool->regions[3].used_count = 0;
    pool->regions[3].peak_usage = 0;

    /* 初始化所有元数据 */
    for (i = 0; i < MEM_POOL_TOTAL_BLOCKS; i++) {
        pool->meta[i].is_used = FALSE;
        pool->meta[i].block_type = 0;
        pool->meta[i].alloc_file = NULL;
        pool->meta[i].alloc_line = 0;
    }

    /* 初始化所有区域的空闲链表 */
    for (i = 0; i < 4; i++) {
        init_free_list(pool, i);
    }

    pool->peak_usage = 0;
    pool->is_initialized = TRUE;

    EXIT_CRITICAL();

    return ERR_OK;
}

void* aegis_mem_pool_alloc(AegisMemPool* pool, uint32_t size, const char* file, uint32_t line) {
    uint8_t* block_start;
    void* user_ptr;
    uint8_t region;
    uint8_t block_idx;
    uint8_t meta_idx;
    uint8_t total_used;

    if (pool == NULL || !pool->is_initialized) {
        return NULL;
    }

    if (size == 0) {
        return NULL;
    }

    block_start = NULL;
    user_ptr = NULL;

    ENTER_CRITICAL();

    /* 选择合适的区域（从小到大尝试） */
    for (region = 0; region < 4; region++) {
        /* 预留头尾魔法数字节 */
        if (size <= ((uint32_t)pool->regions[region].block_size - (uint32_t)(2U * MEM_MAGIC_SIZE))) {
            /* 从空闲链表分配 - O(1) */
            block_idx = alloc_from_free_list(pool, region);

            if (block_idx != FREE_LIST_END) {
                /* 找到空闲块 */
                meta_idx = get_meta_index(pool, region, block_idx);

                pool->meta[meta_idx].is_used = TRUE;
                pool->meta[meta_idx].block_type = region;
                pool->meta[meta_idx].alloc_file = file;
                pool->meta[meta_idx].alloc_line = line;

                block_start = (pool->regions[region].start_addr +
                               (uint32_t)block_idx * (uint32_t)pool->regions[region].block_size);

                /* 写入魔法数保护 */
                write_magic_numbers(block_start, pool->regions[region].block_size);

                /* 返回用户可用指针（跳过头部魔法数） */
                user_ptr = (void*)(block_start + MEM_MAGIC_SIZE);

                /* 更新统计 */
                pool->regions[region].used_count++;
                if (pool->regions[region].used_count > pool->regions[region].peak_usage) {
                    pool->regions[region].peak_usage = pool->regions[region].used_count;
                }

                /* 更新全局峰值 */
                total_used = (uint8_t)(pool->regions[0].used_count + pool->regions[1].used_count +
                             pool->regions[2].used_count + pool->regions[3].used_count);
                if (total_used > pool->peak_usage) {
                    pool->peak_usage = total_used;
                }

                break;  /* 已找到 */
            }
        }
    }

    EXIT_CRITICAL();

    /* 记录分配事件 */
    if (user_ptr != NULL && pool->trace != NULL) {
        ulong_t ptr_val = (ulong_t)user_ptr;
        aegis_trace_log_event(pool->trace, TRACE_EVENT_MEM_ALLOC, "MEM-ALLOC",
                        (uint32_t)(ptr_val & 0xFFFFFFFF), size);
    }

    return user_ptr;
}

AegisErrorCode aegis_mem_pool_free(AegisMemPool* pool, void* ptr) {
    AegisErrorCode ret = ERR_MEM_POOL_INVALID;
    int8_t region;
    uint8_t block_idx;
    uint8_t meta_idx;
    uint8_t* block_start;
    void* expected_user_ptr;

    if (pool == NULL) {
        return ERR_NULL_PTR;
    }

    if (ptr == NULL) {
        return ERR_NULL_PTR;
    }

    if (!pool->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    if ((ulong_t)ptr < (ulong_t)MEM_MAGIC_SIZE) {
        return ERR_MEM_POOL_INVALID;
    }

    block_start = ((uint8_t*)ptr) - MEM_MAGIC_SIZE;

    ENTER_CRITICAL();

    /* 查找指针所属区域 */
    region = find_block_region(pool, block_start, &block_idx);
    if (region >= 0) {
        expected_user_ptr = (void*)(pool->regions[region].start_addr +
                                    (uint32_t)block_idx * (uint32_t)pool->regions[region].block_size +
                                    (uint32_t)MEM_MAGIC_SIZE);
        if (ptr != expected_user_ptr) {
            EXIT_CRITICAL();
            return ERR_MEM_POOL_INVALID;
        }

        meta_idx = get_meta_index(pool, (uint8_t)region, block_idx);

        if (pool->meta[meta_idx].is_used) {
            /* 释放前检查魔法数 */
            if (!check_magic_numbers(block_start, pool->regions[region].block_size)) {
                EXIT_CRITICAL();
                /* 魔法数损坏，记录错误 */
                if (pool->trace != NULL) {
                    aegis_trace_log_event(pool->trace, TRACE_EVENT_MEM_FREE, "MEM-CORRUPT",
                                    (uint32_t)((ulong_t)ptr & 0xFFFFFFFF), 0);
                }
                return ERR_MEM_POOL_INVALID;  /* 内存块已损坏 */
            }

            /* 释放块 */
            pool->meta[meta_idx].is_used = FALSE;
            pool->meta[meta_idx].alloc_file = NULL;
            pool->meta[meta_idx].alloc_line = 0;

            /* 归还到空闲链表 - O(1) */
            free_to_list(pool, (uint8_t)region, block_idx);

            /* 更新统计 */
            pool->regions[region].used_count--;

            ret = ERR_OK;
        } else {
            ret = ERR_MEM_POOL_DOUBLE_FREE;
        }
    }

    EXIT_CRITICAL();

    /* 记录释放事件 */
    if (ret == ERR_OK && pool->trace != NULL) {
        ulong_t ptr_val = (ulong_t)ptr;
        aegis_trace_log_event(pool->trace, TRACE_EVENT_MEM_FREE, "MEM-FREE",
                        (uint32_t)(ptr_val & 0xFFFFFFFF), 0);
    }

    return ret;
}

AegisErrorCode aegis_mem_pool_get_stats(const AegisMemPool* pool, AegisMemPoolStats* stats) {
    uint8_t total_used;

    if (pool == NULL) {
        return ERR_NULL_PTR;
    }

    if (stats == NULL) {
        return ERR_NULL_PTR;
    }

    if (!pool->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    ENTER_CRITICAL();

    total_used = (uint8_t)(pool->regions[0].used_count + pool->regions[1].used_count +
                 pool->regions[2].used_count + pool->regions[3].used_count);

    stats->total_blocks = MEM_POOL_TOTAL_BLOCKS;
    stats->used_blocks = total_used;
    stats->free_blocks = stats->total_blocks - total_used;
    stats->peak_usage = pool->peak_usage;

    stats->small_used = pool->regions[0].used_count;
    stats->medium_used = pool->regions[1].used_count;
    stats->large_used = pool->regions[2].used_count;
    stats->xlarge_used = pool->regions[3].used_count;

    EXIT_CRITICAL();

    return ERR_OK;
}

const void* aegis_mem_pool_get_base_addr(const AegisMemPool* pool) {
    if (pool == NULL) {
        return NULL;
    }
    return (const void*)pool->buffer;
}

uint32_t aegis_mem_pool_get_total_size(void) {
    return MEM_POOL_TOTAL_SIZE;
}

AegisErrorCode aegis_mem_pool_check_magic(const AegisMemPool* pool, const void* ptr) {
    int8_t region;
    uint8_t block_idx;
    const uint8_t* block_start;
    const void* expected_user_ptr;

    if (pool == NULL) {
        return ERR_NULL_PTR;
    }

    if (ptr == NULL) {
        return ERR_NULL_PTR;
    }

    if (!pool->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    ENTER_CRITICAL();

    if ((ulong_t)ptr < (ulong_t)MEM_MAGIC_SIZE) {
        EXIT_CRITICAL();
        return ERR_MEM_POOL_INVALID;
    }

    block_start = ((const uint8_t*)ptr) - MEM_MAGIC_SIZE;

    region = find_block_region(pool, block_start, &block_idx);
    if (region >= 0) {
        expected_user_ptr = (const void*)(pool->regions[region].start_addr +
                                          (uint32_t)block_idx * (uint32_t)pool->regions[region].block_size +
                                          (uint32_t)MEM_MAGIC_SIZE);
        if (ptr != expected_user_ptr) {
            EXIT_CRITICAL();
            return ERR_MEM_POOL_INVALID;
        }

        if (check_magic_numbers(block_start, pool->regions[region].block_size)) {
            EXIT_CRITICAL();
            return ERR_OK;
        }
    }

    EXIT_CRITICAL();
    return ERR_MEM_POOL_INVALID;
}

AegisErrorCode aegis_mem_pool_check_all_magic(const AegisMemPool* pool, uint8_t* corrupted_count) {
    uint8_t region;
    uint8_t i;
    uint8_t meta_idx;
    uint8_t corrupted = 0;

    if (pool == NULL) {
        return ERR_NULL_PTR;
    }

    if (corrupted_count == NULL) {
        return ERR_NULL_PTR;
    }

    if (!pool->is_initialized) {
        return ERR_NOT_INITIALIZED;
    }

    ENTER_CRITICAL();

    for (region = 0; region < 4; region++) {
        for (i = 0; i < pool->regions[region].block_count; i++) {
            meta_idx = get_meta_index(pool, region, i);

            if (pool->meta[meta_idx].is_used) {
                uint8_t* block_addr = pool->regions[region].start_addr +
                                      (uint32_t)i * (uint32_t)pool->regions[region].block_size;
                if (!check_magic_numbers(block_addr, pool->regions[region].block_size)) {
                    corrupted++;
                }
            }
        }
    }

    *corrupted_count = corrupted;

    EXIT_CRITICAL();

    return (corrupted > 0) ? ERR_MEM_POOL_INVALID : ERR_OK;
}
