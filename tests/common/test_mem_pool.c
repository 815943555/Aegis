/*
 * @file: test_mem_pool.c
 * @brief: 内存池模块单元测试
 * @author: jack liu
 * @req: REQ-TEST-MEM-POOL
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mem_pool.h"
#include "trace.h"

/* ==================== 函数原型声明 ==================== */
static void test_mem_pool_init(void);
static void test_mem_pool_alloc_free(void);
static void test_mem_pool_stats(void);
static void test_mem_pool_overflow_detection(void);
static void test_mem_pool_exhaustion(void);

/* ==================== 测试用例计数 ==================== */
static int g_test_passed = 0;
static int g_test_failed = 0;

static uint32_t test_now_ms(void* ctx) {
    (void)ctx;
    return 1U;
}

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            g_test_passed++; \
            printf("  ✓ %s\n", message); \
        } else { \
            g_test_failed++; \
            printf("  ✗ %s (FAILED at %s:%d)\n", message, __FILE__, __LINE__); \
        } \
    } while(0)

/* ==================== 测试用例 ==================== */

/*
 * @test: 测试内存池初始化
 * @req: REQ-TEST-001
 */
static void test_mem_pool_init(void) {
    AegisErrorCode ret;
    AegisTraceLog trace;
    AegisMemPool pool;

    printf("\n[TEST] test_mem_pool_init\n");

    ret = aegis_trace_log_init(&trace, test_now_ms, NULL);
    TEST_ASSERT(ret == ERR_OK, "追溯日志初始化成功");

    ret = aegis_mem_pool_init(&pool, &trace);
    TEST_ASSERT(ret == ERR_OK, "内存池初始化成功");

    /* 重复初始化应该是幂等的 */
    ret = aegis_mem_pool_init(&pool, &trace);
    TEST_ASSERT(ret == ERR_OK, "内存池重复初始化幂等");
}

/*
 * @test: 测试内存分配和释放
 * @req: REQ-TEST-002
 */
static void test_mem_pool_alloc_free(void) {
    void* ptr1;
    void* ptr2;
    AegisErrorCode ret;
    AegisTraceLog trace;
    AegisMemPool pool;

    printf("\n[TEST] test_mem_pool_alloc_free\n");

    (void)aegis_trace_log_init(&trace, test_now_ms, NULL);
    (void)aegis_mem_pool_init(&pool, &trace);

    /* 分配小块 */
    ptr1 = MEM_ALLOC(&pool, 16);
    TEST_ASSERT(ptr1 != NULL, "分配16字节成功");

    /* 分配中块 */
    ptr2 = MEM_ALLOC(&pool, 48);
    TEST_ASSERT(ptr2 != NULL, "分配48字节成功");

    /* 写入数据验证 */
    memset(ptr1, 0xAA, 16);
    memset(ptr2, 0xBB, 48);

    /* 检查魔法数 */
    ret = aegis_mem_pool_check_magic(&pool, ptr1);
    TEST_ASSERT(ret == ERR_OK, "ptr1魔法数完整");

    ret = aegis_mem_pool_check_magic(&pool, ptr2);
    TEST_ASSERT(ret == ERR_OK, "ptr2魔法数完整");

    /* 释放内存 */
    ret = MEM_FREE(&pool, ptr1);
    TEST_ASSERT(ret == ERR_OK, "释放ptr1成功");

    ret = MEM_FREE(&pool, ptr2);
    TEST_ASSERT(ret == ERR_OK, "释放ptr2成功");

    /* 重复释放检测 */
    ret = MEM_FREE(&pool, ptr1);
    TEST_ASSERT(ret == ERR_MEM_POOL_DOUBLE_FREE, "检测到重复释放");
}

/*
 * @test: 测试内存池统计
 * @req: REQ-TEST-003
 */
static void test_mem_pool_stats(void) {
    AegisMemPoolStats stats;
    AegisErrorCode ret;
    void* ptr;
    AegisTraceLog trace;
    AegisMemPool pool;

    printf("\n[TEST] test_mem_pool_stats\n");

    /* 重新初始化 */
    (void)aegis_trace_log_init(&trace, test_now_ms, NULL);
    (void)aegis_mem_pool_init(&pool, &trace);

    /* 获取初始统计 */
    ret = aegis_mem_pool_get_stats(&pool, &stats);
    TEST_ASSERT(ret == ERR_OK, "获取统计信息成功");
    TEST_ASSERT(stats.used_blocks == 0, "初始已用块数为0");

    /* 分配一些内存 */
    ptr = MEM_ALLOC(&pool, 20);
    TEST_ASSERT(ptr != NULL, "分配内存成功");

    /* 再次获取统计 */
    ret = aegis_mem_pool_get_stats(&pool, &stats);
    TEST_ASSERT(ret == ERR_OK, "获取统计信息成功");
    TEST_ASSERT(stats.used_blocks == 1, "已用块数为1");
    TEST_ASSERT(stats.small_used == 1, "小块已用数为1");

    /* 释放 */
    MEM_FREE(&pool, ptr);

    ret = aegis_mem_pool_get_stats(&pool, &stats);
    TEST_ASSERT(ret == ERR_OK, "获取统计信息成功");
    TEST_ASSERT(stats.used_blocks == 0, "释放后已用块数为0");
}

/*
 * @test: 测试内存溢出检测
 * @req: REQ-TEST-004
 */
static void test_mem_pool_overflow_detection(void) {
    void* ptr;
    AegisErrorCode ret;
    uint8_t* byte_ptr;
    AegisTraceLog trace;
    AegisMemPool pool;

    printf("\n[TEST] test_mem_pool_overflow_detection\n");

    /* 重新初始化 */
    (void)aegis_trace_log_init(&trace, test_now_ms, NULL);
    (void)aegis_mem_pool_init(&pool, &trace);

    ptr = MEM_ALLOC(&pool, 16);
    TEST_ASSERT(ptr != NULL, "分配内存成功");

    /* 故意写入超出范围(破坏尾部魔法数) */
    byte_ptr = (uint8_t*)ptr;
    /* 注意:小块大小是32字节,头尾魔法数各2字节,用户可用28字节 */
    /* 写入到尾部魔法数位置（破坏） */
    byte_ptr[28] = 0xFF;  /* 破坏尾部魔法数 */

    /* 检查魔法数应该失败 */
    ret = aegis_mem_pool_check_magic(&pool, ptr);
    TEST_ASSERT(ret == ERR_MEM_POOL_INVALID, "检测到内存溢出");
}

/*
 * @test: 测试内存池耗尽
 * @req: REQ-TEST-005
 */
static void test_mem_pool_exhaustion(void) {
    void* ptrs[40];
    void* extra;  /* C89: 所有变量必须在块开头声明 */
    int i;
    int allocated = 0;
    AegisTraceLog trace;
    AegisMemPool pool;

    printf("\n[TEST] test_mem_pool_exhaustion\n");

    /* 重新初始化 */
    (void)aegis_trace_log_init(&trace, test_now_ms, NULL);
    (void)aegis_mem_pool_init(&pool, &trace);

    /* 尝试分配所有可用块（按最小适配策略：小块耗尽后会继续使用更大块） */
    for (i = 0; i < (int)(sizeof(ptrs) / sizeof(ptrs[0])); i++) {
        ptrs[i] = MEM_ALLOC(&pool, 16);
        if (ptrs[i] == NULL) {
            break;
        }
        allocated++;
    }

    TEST_ASSERT(allocated == (int)MEM_POOL_TOTAL_BLOCKS, "分配数量等于内存池总块数");

    /* 内存池已耗尽：再次分配应失败 */
    extra = MEM_ALLOC(&pool, 16);  /* C89: 变量已在函数开头声明 */
    TEST_ASSERT(extra == NULL, "耗尽后再次分配返回NULL");

    /* 清理 */
    for (i = 0; i < allocated; i++) {
        if (ptrs[i] != NULL) {
            MEM_FREE(&pool, ptrs[i]);
        }
    }
}

/* ==================== 测试入口 ==================== */
int main(void) {
    printf("========================================\n");
    printf("  内存池单元测试\n");
    printf("========================================\n");

    /* 运行所有测试 */
    test_mem_pool_init();
    test_mem_pool_alloc_free();
    test_mem_pool_stats();
    test_mem_pool_overflow_detection();
    test_mem_pool_exhaustion();

    /* 输出测试结果 */
    printf("\n========================================\n");
    printf("测试结果:\n");
    printf("  通过: %d\n", g_test_passed);
    printf("  失败: %d\n", g_test_failed);
    printf("========================================\n");

    if (g_test_failed == 0) {
        printf("✅ 所有测试通过!\n");
        return 0;
    } else {
        printf("❌ 存在失败的测试!\n");
        return 1;
    }
}
