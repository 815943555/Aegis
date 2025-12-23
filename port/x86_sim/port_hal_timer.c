/*
 * @file: port_hal_timer.c
 * @brief: x86_sim平台定时器模拟实现
 * @author: jack liu
 */

#include "hal_timer.h"
#include "critical.h"
#include <string.h>

/* x86_sim 使用简单计数器代替系统时钟 */
static uint32_t g_tick_counter = 0;

/* ==================== 模拟定时器状态 ==================== */
#define MAX_TIMERS  4

typedef struct {
    bool_t initialized;
    bool_t running;
    uint32_t period_us;
    HalTimerCallback callback;
} TimerState;

static TimerState g_timer_state[MAX_TIMERS];
static bool_t g_timer_hal_initialized = FALSE;

/* ==================== 内部辅助函数 ==================== */
static AegisErrorCode validate_timer(AegisHalTimerId timer_id) {
    if (timer_id >= MAX_TIMERS) {
        return ERR_INVALID_PARAM;
    }
    return ERR_OK;
}

/* ==================== 公共接口实现 ==================== */
AegisErrorCode aegis_hal_timer_init(const AegisHalTimerConfig* config) {
    AegisErrorCode ret;

    if (config == NULL) {
        return ERR_NULL_PTR;
    }

    if (config->callback == NULL) {
        return ERR_NULL_PTR;
    }

    ret = validate_timer(config->timer_id);
    if (ret != ERR_OK) {
        return ret;
    }

    if (config->period_us == 0) {
        return ERR_INVALID_PARAM;
    }

    ENTER_CRITICAL();

    /* 初始化全局状态（首次调用） */
    if (!g_timer_hal_initialized) {
        memset(g_timer_state, 0, sizeof(g_timer_state));
        g_timer_hal_initialized = TRUE;
    }

    /* 配置定时器 */
    g_timer_state[config->timer_id].initialized = TRUE;
    g_timer_state[config->timer_id].running = FALSE;
    g_timer_state[config->timer_id].period_us = config->period_us;
    g_timer_state[config->timer_id].callback = config->callback;

    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_hal_timer_start(AegisHalTimerId timer_id) {
    AegisErrorCode ret;

    ret = validate_timer(timer_id);
    if (ret != ERR_OK) {
        return ret;
    }

    if (!g_timer_state[timer_id].initialized) {
        return ERR_NOT_INITIALIZED;
    }

    ENTER_CRITICAL();
    g_timer_state[timer_id].running = TRUE;
    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_hal_timer_stop(AegisHalTimerId timer_id) {
    AegisErrorCode ret;

    ret = validate_timer(timer_id);
    if (ret != ERR_OK) {
        return ret;
    }

    if (!g_timer_state[timer_id].initialized) {
        return ERR_NOT_INITIALIZED;
    }

    ENTER_CRITICAL();
    g_timer_state[timer_id].running = FALSE;
    EXIT_CRITICAL();

    return ERR_OK;
}

uint32_t aegis_hal_timer_get_tick_ms(void) {
    uint32_t tick;

    ENTER_CRITICAL();
    /* x86_sim使用简单计数器模拟 */
    g_tick_counter++;
    tick = g_tick_counter;
    EXIT_CRITICAL();

    return tick;
}
