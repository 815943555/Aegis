/*
 * @file: aegis_app_init.h
 * @brief: 应用层初始化接口
 * @author: jack liu
 * @req: REQ-APP-020
 * @design: DES-APP-020
 * @asil: ASIL-B
 */

#ifndef APP_INIT_H
#define APP_INIT_H

#include "types.h"
#include "error_codes.h"
#include "domain_event.h"
#include "domain_repository_write.h"
#include "app_command.h"
#include "app_cmd_service.h"
#include "app_query.h"
#include "app_domain_assembler.h"
#include "app_domain_converter.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    AegisTraceLog* trace;
    const AegisDomainRepositoryWriteInterface* write_repo;
    const AegisEventSubscription* event_subscriptions;
    uint8_t event_subscription_count;
} AegisAppInitConfig;

typedef struct {
    AegisTraceLog* trace;
    const AegisDomainRepositoryWriteInterface* write_repo;

    AegisDomainEventBus event_bus;
    AegisAppCmdQueue cmd_queue;
    AegisAppCmdService cmd_service;
    AegisAppQueryDispatcher query;
    AegisAppAssembler assembler;
    AegisAppConverter converter;

    bool_t is_initialized;
} AegisAppRuntime;

/* ==================== 应用层初始化接口 ==================== */
/*
 * @brief: 初始化应用层(包含领域层和命令查询系统)
 * @param runtime: 应用运行时实例
 * @param config: 初始化配置（依赖注入）
 * @return: 错误码
 * @req: REQ-APP-021
 * @design: DES-APP-021
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_app_init_all(AegisAppRuntime* runtime, const AegisAppInitConfig* config);

/*
 * @brief: 处理异步领域事件队列（主循环调用）
 * @param max_events: 本次最多处理的事件数量（0=处理所有）
 * @return: 实际处理的事件数量
 * @req: REQ-APP-041
 * @design: DES-APP-041
 * @asil: ASIL-B
 * @isr_unsafe
 */
uint8_t aegis_app_init_process_domain_events(AegisAppRuntime* runtime, uint8_t max_events);

#ifdef __cplusplus
}
#endif

#endif /* APP_INIT_H */
