/*
 * @file: main.c
 * @brief: x86_sim 示例入口
 * @author: jack liu
 */

#include "entry_init.h"
#include "entry_main.h"
#include "entry_platform.h"
#include "hal_timer.h"

static uint32_t now_ms(void* ctx) {
    (void)ctx;
    return aegis_hal_timer_get_tick_ms();
}

int main(void) {
    AegisEntryRuntime runtime;
    AegisEntryConfig cfg;
    const AegisDomainRepositoryWriteInterface* write_repo;
    AegisErrorCode ret;

    ret = aegis_entry_platform_init(now_ms, NULL);
    if (ret != ERR_OK) {
        return (int)ret;
    }
    write_repo = aegis_entry_platform_get_write_repo();
    if (write_repo == NULL) {
        return (int)ERR_NULL_PTR;
    }

    cfg.aegis_trace_now_fn = now_ms;
    cfg.aegis_trace_now_ctx = NULL;
    cfg.write_repo = write_repo;
    cfg.event_subscriptions = NULL;
    cfg.event_subscription_count = 0;

    ret = aegis_entry_init_all(&runtime, &cfg);
    if (ret != ERR_OK) {
        return (int)ret;
    }

    for (;;) {
        (void)aegis_entry_main_loop_once(&runtime);
    }

    /* never reached */
    return 0;
}
