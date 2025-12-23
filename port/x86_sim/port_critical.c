/*
 * @file: port_critical.c
 * @brief: x86 模拟平台临界区实现（空操作）
 * @author: jack liu
 */

#include "critical.h"

/*
 * x86 模拟平台下，临界区操作为空
 * 实际 MCU 平台需要实现关中断/开中断操作
 */

void aegis_critical_enter(void) {
    /* x86 模拟环境：单线程，无需关中断 */
}

void aegis_critical_exit(void) {
    /* x86 模拟环境：单线程，无需开中断 */
}
