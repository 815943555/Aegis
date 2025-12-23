/*
 * @file: aegis_entry_main.c
 * @brief: 主循环实现（严格依赖注入/实例化）
 * @author: jack liu
 */

#include "entry_main.h"
#include <string.h>

AegisErrorCode aegis_entry_main_loop_once(AegisEntryRuntime* runtime) {
    AegisErrorCode ret;
    AegisCommand cmd;
    AegisCommandResult result;
    uint8_t cmd_count;

    if (runtime == NULL) {
        return ERR_NULL_PTR;
    }

    if (!aegis_entry_is_initialized(runtime)) {
        return ERR_NOT_INITIALIZED;
    }

    ret = aegis_app_cmd_get_count(&runtime->app.cmd_queue, &cmd_count);
    if (ret != ERR_OK) {
        return ret;
    }

    if (cmd_count > 0U) {
        memset(&cmd, 0, sizeof(AegisCommand));
        ret = aegis_app_cmd_dequeue(&runtime->app.cmd_queue, &cmd);
        if (ret == ERR_OK) {
            ret = aegis_app_cmd_service_execute(&runtime->app.cmd_service, &cmd, &result);
            aegis_trace_log_event(&runtime->trace, TRACE_EVENT_CMD_EXEC, "CMD-EXEC",
                            (uint32_t)cmd.type, (uint32_t)ret);

            if (ret != ERR_OK) {
                aegis_trace_log_event(&runtime->trace, TRACE_EVENT_CMD_EXEC, "CMD-EXEC-ERR",
                                (uint32_t)ret, 0);
            }
        }
    }

    (void)aegis_app_init_process_domain_events(&runtime->app, 4);
    return ERR_OK;
}

AegisErrorCode aegis_entry_main_loop(AegisEntryRuntime* runtime) {
    AegisErrorCode ret;

    if (runtime == NULL) {
        return ERR_NULL_PTR;
    }

    if (!aegis_entry_is_initialized(runtime)) {
        return ERR_NOT_INITIALIZED;
    }

    while (TRUE) {
        ret = aegis_entry_main_loop_once(runtime);
        if (ret != ERR_OK) {
            aegis_trace_log_event(&runtime->trace, TRACE_EVENT_SYSTEM_ERROR, "MAIN-LOOP-ERR",
                            (uint32_t)ret, 0);
        }
    }

    return ERR_OK;
}

