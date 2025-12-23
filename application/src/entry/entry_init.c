/*
 * @file: aegis_entry_init.c
 * @brief: 系统初始化实现（严格依赖注入/实例化）
 * @author: jack liu
 */

#include "entry_init.h"
#include <string.h>

AegisErrorCode aegis_entry_init_all(AegisEntryRuntime* runtime, const AegisEntryConfig* config) {
    AegisErrorCode ret;
    AegisAppInitConfig aegis_app_cfg;

    if (runtime == NULL || config == NULL) {
        return ERR_NULL_PTR;
    }

    memset(runtime, 0, sizeof(AegisEntryRuntime));

    /* 1. 初始化追溯日志 */
    ret = aegis_trace_log_init(&runtime->trace, config->aegis_trace_now_fn, config->aegis_trace_now_ctx);
    if (ret != ERR_OK) {
        return ret;
    }

    /* 2. 初始化内存池 */
    ret = aegis_mem_pool_init(&runtime->mem_pool, &runtime->trace);
    if (ret != ERR_OK) {
        return ret;
    }

    /* 3. 初始化应用层（注入仓储/事件订阅/trace） */
    if (config->write_repo == NULL) {
        return ERR_NULL_PTR;
    }
    aegis_app_cfg.trace = &runtime->trace;
    aegis_app_cfg.write_repo = config->write_repo;
    aegis_app_cfg.event_subscriptions = config->event_subscriptions;
    aegis_app_cfg.event_subscription_count = config->event_subscription_count;

    ret = aegis_app_init_all(&runtime->app, &aegis_app_cfg);
    if (ret != ERR_OK) {
        return ret;
    }

    runtime->is_initialized = TRUE;

    aegis_trace_log_event(&runtime->trace, TRACE_EVENT_SYSTEM_INIT, "SYSTEM-INIT-OK", 0, 0);

    return ERR_OK;
}

bool_t aegis_entry_is_initialized(const AegisEntryRuntime* runtime) {
    if (runtime == NULL) {
        return FALSE;
    }
    return runtime->is_initialized;
}
