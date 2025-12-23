/*
 * @file: port_critical.c
 * @brief: STM32F030 临界区实现（PRIMASK + 嵌套）
 * @author: jack liu
 *
 * @note:
 * - 该实现为移植示例：在 Cortex-M0 上使用 PRIMASK 屏蔽中断。
 * - 若你的工程使用 CMSIS，可用 __get_PRIMASK/__set_PRIMASK/__disable_irq/__enable_irq 替换汇编。
 */

#include "types.h"
#include "critical.h"

static uint32_t g_saved_primask = 0U;
static uint8_t g_nest = 0U;

static uint32_t read_primask(void) {
    uint32_t v;
    v = 0U;
#if defined(__arm__) || defined(__thumb__)
    __asm volatile("mrs %0, primask" : "=r"(v) :: "memory");
#endif
    return v;
}

static void write_primask(uint32_t v) {
#if defined(__arm__) || defined(__thumb__)
    __asm volatile("msr primask, %0" :: "r"(v) : "memory");
#else
    (void)v;
#endif
}

static void disable_irq(void) {
#if defined(__arm__) || defined(__thumb__)
    __asm volatile("cpsid i" ::: "memory");
#endif
}

void aegis_critical_enter(void) {
    uint32_t primask;

    primask = read_primask();
    disable_irq();

    if (g_nest == 0U) {
        g_saved_primask = primask;
    }

    if (g_nest < (uint8_t)0xFF) {
        g_nest++;
    }
}

void aegis_critical_exit(void) {
    if (g_nest == 0U) {
        return;
    }

    g_nest--;
    if (g_nest == 0U) {
        write_primask(g_saved_primask);
    }
}

