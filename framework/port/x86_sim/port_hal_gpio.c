/*
 * @file: port_hal_gpio.c
 * @brief: x86_sim平台GPIO模拟实现
 * @author: jack liu
 */

#include "hal_gpio.h"
#include "critical.h"
#include <string.h>

/* ==================== 模拟GPIO状态 ==================== */
#define MAX_GPIO_PORTS  8   /* 最多8个端口 (A-H) */
#define MAX_GPIO_PINS   16  /* 每个端口16个引脚 */

typedef struct {
    AegisHalGpioMode mode;
    AegisHalGpioPull pull;
    AegisHalGpioLevel level;
    bool_t initialized;
} GpioState;

static GpioState g_gpio_state[MAX_GPIO_PORTS][MAX_GPIO_PINS];
static bool_t g_gpio_hal_initialized = FALSE;

/* ==================== 内部辅助函数 ==================== */
static AegisErrorCode validate_pin(uint8_t port, uint8_t pin) {
    if (port >= MAX_GPIO_PORTS) {
        return ERR_INVALID_PARAM;
    }
    if (pin >= MAX_GPIO_PINS) {
        return ERR_INVALID_PARAM;
    }
    return ERR_OK;
}

/* ==================== 公共接口实现 ==================== */
AegisErrorCode aegis_hal_gpio_init(const AegisHalGpioConfig* config) {
    AegisErrorCode ret;

    if (config == NULL) {
        return ERR_NULL_PTR;
    }

    ret = validate_pin(config->port, config->pin);
    if (ret != ERR_OK) {
        return ret;
    }

    ENTER_CRITICAL();

    /* 初始化全局状态（首次调用） */
    if (!g_gpio_hal_initialized) {
        memset(g_gpio_state, 0, sizeof(g_gpio_state));
        g_gpio_hal_initialized = TRUE;
    }

    /* 配置引脚 */
    g_gpio_state[config->port][config->pin].mode = config->mode;
    g_gpio_state[config->port][config->pin].pull = config->pull;
    g_gpio_state[config->port][config->pin].level = HAL_GPIO_LEVEL_LOW;
    g_gpio_state[config->port][config->pin].initialized = TRUE;

    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_hal_gpio_write(uint8_t port, uint8_t pin, AegisHalGpioLevel level) {
    AegisErrorCode ret;

    ret = validate_pin(port, pin);
    if (ret != ERR_OK) {
        return ret;
    }

    if (!g_gpio_state[port][pin].initialized) {
        return ERR_NOT_INITIALIZED;
    }

    if (g_gpio_state[port][pin].mode != HAL_GPIO_MODE_OUTPUT) {
        return ERR_INVALID_STATE;
    }

    ENTER_CRITICAL();
    g_gpio_state[port][pin].level = level;
    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_hal_gpio_read(uint8_t port, uint8_t pin, AegisHalGpioLevel* level) {
    AegisErrorCode ret;

    if (level == NULL) {
        return ERR_NULL_PTR;
    }

    ret = validate_pin(port, pin);
    if (ret != ERR_OK) {
        return ret;
    }

    if (!g_gpio_state[port][pin].initialized) {
        return ERR_NOT_INITIALIZED;
    }

    ENTER_CRITICAL();
    *level = g_gpio_state[port][pin].level;
    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_hal_gpio_toggle(uint8_t port, uint8_t pin) {
    AegisErrorCode ret;

    ret = validate_pin(port, pin);
    if (ret != ERR_OK) {
        return ret;
    }

    if (!g_gpio_state[port][pin].initialized) {
        return ERR_NOT_INITIALIZED;
    }

    if (g_gpio_state[port][pin].mode != HAL_GPIO_MODE_OUTPUT) {
        return ERR_INVALID_STATE;
    }

    ENTER_CRITICAL();
    g_gpio_state[port][pin].level = (g_gpio_state[port][pin].level == HAL_GPIO_LEVEL_LOW)
                                      ? HAL_GPIO_LEVEL_HIGH : HAL_GPIO_LEVEL_LOW;
    EXIT_CRITICAL();

    return ERR_OK;
}
