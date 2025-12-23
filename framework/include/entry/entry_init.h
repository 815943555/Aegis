/*
 * @file: aegis_entry_init.h
 * @brief: 系统初始化入口
 * @author: jack liu
 * @req: REQ-ENTRY-001
 * @design: DES-ENTRY-001
 * @asil: ASIL-B
 */

#ifndef ENTRY_INIT_H
#define ENTRY_INIT_H

#include "types.h"
#include "error_codes.h"
#include "mem_pool.h"
#include "trace.h"
#include "app_init.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    TraceTimestampFn aegis_trace_now_fn;
    void* aegis_trace_now_ctx;

    /* 由组合根注入：entry 不直接依赖/创建 Infrastructure */
    const AegisDomainRepositoryWriteInterface* write_repo;

    const AegisEventSubscription* event_subscriptions;
    uint8_t event_subscription_count;
} AegisEntryConfig;

typedef struct {
    AegisMemPool mem_pool;
    AegisTraceLog trace;
    AegisAppRuntime app;
    bool_t is_initialized;
} AegisEntryRuntime;

/* ==================== 系统初始化接口 ==================== */
/*
 * @brief: 初始化所有子系统
 * @param runtime: 入口运行时实例
 * @param config: 入口配置（依赖注入）
 * @return: 错误码
 * @req: REQ-ENTRY-002
 * @design: DES-ENTRY-002
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_entry_init_all(AegisEntryRuntime* runtime, const AegisEntryConfig* config);

/*
 * @brief: 获取系统初始化状态
 * @param runtime: 入口运行时实例
 * @return: TRUE=已初始化，FALSE=未初始化
 * @req: REQ-ENTRY-003
 * @design: DES-ENTRY-003
 * @asil: ASIL-B
 * @isr_safe
 */
bool_t aegis_entry_is_initialized(const AegisEntryRuntime* runtime);

#ifdef __cplusplus
}
#endif

#endif /* ENTRY_INIT_H */
