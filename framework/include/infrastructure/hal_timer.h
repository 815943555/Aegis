/*
 * @file: aegis_hal_timer.h
 * @brief: 定时器硬件抽象层接口
 * @author: jack liu
 * @req: REQ-HAL-010
 * @design: DES-HAL-010
 * @asil: ASIL-B
 */

#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include "types.h"
#include "error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 定时器配置 ==================== */
typedef enum {
    HAL_TIMER_0 = 0,
    HAL_TIMER_1,
    HAL_TIMER_2,
    HAL_TIMER_3
} AegisHalTimerId;

typedef void (*HalTimerCallback)(void);

typedef struct {
    AegisHalTimerId timer_id;            /* 定时器ID */
    uint32_t period_us;             /* 周期（微秒） */
    HalTimerCallback callback;      /* 回调函数（ISR中调用） */
} AegisHalTimerConfig;

/* ==================== 定时器接口 ==================== */
/*
 * @brief: 初始化定时器
 * @param config: 定时器配置
 * @return: 错误码
 * @req: REQ-HAL-011
 * @design: DES-HAL-011
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_hal_timer_init(const AegisHalTimerConfig* config);

/*
 * @brief: 启动定时器
 * @param timer_id: 定时器ID
 * @return: 错误码
 * @req: REQ-HAL-012
 * @design: DES-HAL-012
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_hal_timer_start(AegisHalTimerId timer_id);

/*
 * @brief: 停止定时器
 * @param timer_id: 定时器ID
 * @return: 错误码
 * @req: REQ-HAL-013
 * @design: DES-HAL-013
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_hal_timer_stop(AegisHalTimerId timer_id);

/*
 * @brief: 获取系统滴答计数（毫秒）
 * @return: 系统启动以来的毫秒数
 * @req: REQ-HAL-014
 * @design: DES-HAL-014
 * @asil: ASIL-B
 * @isr_safe
 */
uint32_t aegis_hal_timer_get_tick_ms(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_TIMER_H */
