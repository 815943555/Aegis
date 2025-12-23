/*
 * @file: aegis_app_init.c
 * @brief: 应用层初始化实现（严格依赖注入/实例化）
 * @author: jack liu
 * @req: REQ-APP-020
 * @design: DES-APP-020
 * @asil: ASIL-B
 */

#include "app_init.h"
#include <string.h>

AegisErrorCode aegis_app_init_all(AegisAppRuntime* runtime, const AegisAppInitConfig* config) {
    AegisErrorCode ret;
    const AegisDomainRepositoryWriteInterface* write_repo;
    AegisTraceLog* trace;

    if (runtime == NULL || config == NULL) {
        return ERR_NULL_PTR;
    }

    write_repo = config->write_repo;
    trace = config->trace;

    if (write_repo == NULL ||
        write_repo->init == NULL ||
        write_repo->create == NULL ||
        write_repo->update == NULL ||
        write_repo->delete_entity == NULL ||
        write_repo->read.get == NULL ||
        write_repo->read.find_by_type == NULL ||
        write_repo->read.count_by_type == NULL) {
        return ERR_NULL_PTR;
    }

    memset(runtime, 0, sizeof(AegisAppRuntime));
    runtime->trace = trace;
    runtime->write_repo = write_repo;

    /* 1. 初始化领域事件总线 */
    ret = aegis_domain_event_bus_init(&runtime->event_bus,
                                trace,
                                config->event_subscriptions,
                                config->event_subscription_count);
    if (ret != ERR_OK) {
        return ret;
    }

    /* 2. 初始化仓储实现 */
    ret = write_repo->init(write_repo);
    if (ret != ERR_OK) {
        return ret;
    }

    /* 3. 初始化命令系统与应用服务 */
    ret = aegis_app_cmd_service_init(&runtime->cmd_service);
    if (ret != ERR_OK) {
        return ret;
    }

    ret = aegis_app_cmd_init(&runtime->cmd_queue, trace);
    if (ret != ERR_OK) {
        return ret;
    }

    /* 4. 初始化查询系统 */
    ret = aegis_app_query_init(&runtime->query);
    if (ret != ERR_OK) {
        return ret;
    }

    /* 5. 初始化 DTO 组装/转换注册表（可选） */
    ret = aegis_app_asm_init(&runtime->assembler);
    if (ret != ERR_OK) {
        return ret;
    }

    ret = aegis_app_conv_init(&runtime->converter);
    if (ret != ERR_OK) {
        return ret;
    }

    runtime->is_initialized = TRUE;
    return ERR_OK;
}

uint8_t aegis_app_init_process_domain_events(AegisAppRuntime* runtime, uint8_t max_events) {
    if (runtime == NULL || !runtime->is_initialized) {
        return 0U;
    }
    return aegis_domain_event_process(&runtime->event_bus, max_events);
}

