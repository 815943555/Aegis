/*
 * @file: critical.h
 * @brief: 临界区抽象接口
 * @author: jack liu
 * @req: REQ-COMMON-005
 * @design: DES-COMMON-005
 * @asil: ASIL-B
 */

#ifndef CRITICAL_H
#define CRITICAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 临界区接口 ==================== */
/*
 * @brief: 进入临界区（关中断）
 * @req: REQ-CRITICAL-001
 * @design: DES-CRITICAL-001
 * @asil: ASIL-B
 * @isr_safe
 */
void aegis_critical_enter(void);

/*
 * @brief: 退出临界区（开中断）
 * @req: REQ-CRITICAL-002
 * @design: DES-CRITICAL-002
 * @asil: ASIL-B
 * @isr_safe
 */
void aegis_critical_exit(void);

/* 便捷宏定义 */
#define ENTER_CRITICAL()    aegis_critical_enter()
#define EXIT_CRITICAL()     aegis_critical_exit()

#ifdef __cplusplus
}
#endif

#endif /* CRITICAL_H */
