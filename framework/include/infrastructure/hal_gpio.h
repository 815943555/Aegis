/*
 * @file: aegis_hal_gpio.h
 * @brief: GPIO硬件抽象层接口
 * @author: jack liu
 * @req: REQ-HAL-001
 * @design: DES-HAL-001
 * @asil: ASIL-B
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "types.h"
#include "error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== GPIO 配置 ==================== */
typedef enum {
    HAL_GPIO_MODE_INPUT = 0,        /* 输入模式 */
    HAL_GPIO_MODE_OUTPUT,           /* 输出模式 */
    HAL_GPIO_MODE_AF,               /* 复用功能 */
    HAL_GPIO_MODE_ANALOG            /* 模拟模式 */
} AegisHalGpioMode;

typedef enum {
    HAL_GPIO_PULL_NONE = 0,         /* 无上下拉 */
    HAL_GPIO_PULL_UP,               /* 上拉 */
    HAL_GPIO_PULL_DOWN              /* 下拉 */
} AegisHalGpioPull;

typedef enum {
    HAL_GPIO_LEVEL_LOW = 0,         /* 低电平 */
    HAL_GPIO_LEVEL_HIGH             /* 高电平 */
} AegisHalGpioLevel;

typedef struct {
    uint8_t port;                   /* 端口号 (0=A, 1=B, ...) */
    uint8_t pin;                    /* 引脚号 (0-15) */
    AegisHalGpioMode mode;               /* 工作模式 */
    AegisHalGpioPull pull;               /* 上下拉配置 */
} AegisHalGpioConfig;

/* ==================== GPIO 接口 ==================== */
/*
 * @brief: 初始化GPIO引脚
 * @param config: GPIO配置
 * @return: 错误码
 * @req: REQ-HAL-002
 * @design: DES-HAL-002
 * @asil: ASIL-B
 * @isr_unsafe
 */
AegisErrorCode aegis_hal_gpio_init(const AegisHalGpioConfig* config);

/*
 * @brief: 写GPIO输出电平
 * @param port: 端口号
 * @param pin: 引脚号
 * @param level: 输出电平
 * @return: 错误码
 * @req: REQ-HAL-003
 * @design: DES-HAL-003
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_hal_gpio_write(uint8_t port, uint8_t pin, AegisHalGpioLevel level);

/*
 * @brief: 读GPIO输入电平
 * @param port: 端口号
 * @param pin: 引脚号
 * @param level: 输出读取的电平
 * @return: 错误码
 * @req: REQ-HAL-004
 * @design: DES-HAL-004
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_hal_gpio_read(uint8_t port, uint8_t pin, AegisHalGpioLevel* level);

/*
 * @brief: 翻转GPIO输出电平
 * @param port: 端口号
 * @param pin: 引脚号
 * @return: 错误码
 * @req: REQ-HAL-005
 * @design: DES-HAL-005
 * @asil: ASIL-B
 * @isr_safe
 */
AegisErrorCode aegis_hal_gpio_toggle(uint8_t port, uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_H */
