/*
 * @file: port_hal_timer.c
 * @brief: STM32F030 Timer 移植示例（SysTick tick + 软件定时器）
 * @author: jack liu
 *
 * @note:
 * - 本实现以 SysTick 为基础：aegis_hal_timer_get_tick_ms() 通过 COUNTFLAG 轮询累加毫秒计数。
 * - HAL_TIMER_0..3 使用“软件定时器”示例实现：精度按 ms（period_us 向上取整到 1ms）。
 * - 更高精度/更低抖动：可把 HAL_TIMER_x 映射到 TIM14/TIM16/TIM17 等硬件定时器，并在中断里调用回调。
 */

#include "hal_timer.h"
#include "critical.h"
#include <string.h>

/* ==================== SysTick 最小寄存器定义 ==================== */
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} Stm32SysTickRegs;

#define STM32_SYSTICK_BASE (0xE000E010UL)
#define SYSTICK ((Stm32SysTickRegs*)STM32_SYSTICK_BASE)

#define SYSTICK_CTRL_COUNTFLAG (1UL << 16)

/* ==================== 软件定时器状态 ==================== */
typedef struct {
    bool_t initialized;
    bool_t running;
    uint32_t period_ms;
    uint32_t next_fire_ms;
    HalTimerCallback callback;
} TimerState;

static TimerState g_timer_state[4];
static bool_t g_timer_hal_initialized = FALSE;
static uint32_t g_tick_ms = 0U;

static AegisErrorCode validate_timer(AegisHalTimerId timer_id) {
    if ((uint8_t)timer_id >= 4U) {
        return ERR_INVALID_PARAM;
    }
    return ERR_OK;
}

static void update_tick_ms_from_systick(void) {
    /* COUNTFLAG 只记录“至少一次下溢”，若长时间不调用可能丢 tick；建议在 SysTick ISR 中调用一次。 */
    if ((SYSTICK->CTRL & SYSTICK_CTRL_COUNTFLAG) != 0UL) {
        g_tick_ms++;
    }
}

static void process_software_timers(void) {
    uint8_t i;
    uint32_t now;

    now = g_tick_ms;
    for (i = 0; i < 4U; i++) {
        if (g_timer_state[i].running && g_timer_state[i].callback != NULL) {
            if (now >= g_timer_state[i].next_fire_ms) {
                g_timer_state[i].next_fire_ms = now + g_timer_state[i].period_ms;
                g_timer_state[i].callback();
            }
        }
    }
}

AegisErrorCode aegis_hal_timer_init(const AegisHalTimerConfig* config) {
    AegisErrorCode ret;
    uint32_t period_ms;

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

    if (config->period_us == 0U) {
        return ERR_INVALID_PARAM;
    }

    period_ms = (config->period_us + 999U) / 1000U;
    if (period_ms == 0U) {
        period_ms = 1U;
    }

    ENTER_CRITICAL();
    if (!g_timer_hal_initialized) {
        memset(g_timer_state, 0, sizeof(g_timer_state));
        g_timer_hal_initialized = TRUE;
    }

    g_timer_state[(uint8_t)config->timer_id].initialized = TRUE;
    g_timer_state[(uint8_t)config->timer_id].running = FALSE;
    g_timer_state[(uint8_t)config->timer_id].period_ms = period_ms;
    g_timer_state[(uint8_t)config->timer_id].next_fire_ms = g_tick_ms + period_ms;
    g_timer_state[(uint8_t)config->timer_id].callback = config->callback;
    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_hal_timer_start(AegisHalTimerId timer_id) {
    AegisErrorCode ret;

    ret = validate_timer(timer_id);
    if (ret != ERR_OK) {
        return ret;
    }

    if (!g_timer_state[(uint8_t)timer_id].initialized) {
        return ERR_NOT_INITIALIZED;
    }

    ENTER_CRITICAL();
    g_timer_state[(uint8_t)timer_id].running = TRUE;
    g_timer_state[(uint8_t)timer_id].next_fire_ms = g_tick_ms + g_timer_state[(uint8_t)timer_id].period_ms;
    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_hal_timer_stop(AegisHalTimerId timer_id) {
    AegisErrorCode ret;

    ret = validate_timer(timer_id);
    if (ret != ERR_OK) {
        return ret;
    }

    if (!g_timer_state[(uint8_t)timer_id].initialized) {
        return ERR_NOT_INITIALIZED;
    }

    ENTER_CRITICAL();
    g_timer_state[(uint8_t)timer_id].running = FALSE;
    EXIT_CRITICAL();

    return ERR_OK;
}

uint32_t aegis_hal_timer_get_tick_ms(void) {
    ENTER_CRITICAL();
    update_tick_ms_from_systick();
    process_software_timers();
    EXIT_CRITICAL();
    return g_tick_ms;
}

