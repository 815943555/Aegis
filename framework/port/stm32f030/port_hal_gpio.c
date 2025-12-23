/*
 * @file: port_hal_gpio.c
 * @brief: STM32F030 GPIO 寄存器级移植示例
 * @author: jack liu
 *
 * @note:
 * - 端口号约定：0=A, 1=B, 2=C, 3=D, 5=F（F0 系列常见端口）
 * - 该实现仅示例核心寄存器写法，时钟/复用(AF)/速度等可按项目需要扩展。
 */

#include "hal_gpio.h"
#include "critical.h"

/* ==================== STM32F0 寄存器定义（最小子集） ==================== */
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFRL;
    volatile uint32_t AFRH;
    volatile uint32_t BRR;
} Stm32GpioRegs;

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
    volatile uint32_t AHBRSTR;
    volatile uint32_t CFGR2;
    volatile uint32_t CFGR3;
    volatile uint32_t CR2;
} Stm32RccRegs;

#define STM32_RCC_BASE      (0x40021000UL)
#define STM32_GPIOA_BASE    (0x48000000UL)
#define STM32_GPIOB_BASE    (0x48000400UL)
#define STM32_GPIOC_BASE    (0x48000800UL)
#define STM32_GPIOD_BASE    (0x48000C00UL)
#define STM32_GPIOF_BASE    (0x48001400UL)

#define RCC ((Stm32RccRegs*)STM32_RCC_BASE)

static Stm32GpioRegs* gpio_from_port(uint8_t port) {
    if (port == 0U) {
        return (Stm32GpioRegs*)STM32_GPIOA_BASE;
    }
    if (port == 1U) {
        return (Stm32GpioRegs*)STM32_GPIOB_BASE;
    }
    if (port == 2U) {
        return (Stm32GpioRegs*)STM32_GPIOC_BASE;
    }
    if (port == 3U) {
        return (Stm32GpioRegs*)STM32_GPIOD_BASE;
    }
    if (port == 5U) {
        return (Stm32GpioRegs*)STM32_GPIOF_BASE;
    }
    return (Stm32GpioRegs*)0;
}

static void enable_gpio_clock(uint8_t port) {
    /* RCC->AHBENR: IOPAEN=17, IOPBEN=18, IOPCEN=19, IOPDEN=20, IOPFEN=22 */
    if (port == 0U) {
        RCC->AHBENR |= (1UL << 17);
    } else if (port == 1U) {
        RCC->AHBENR |= (1UL << 18);
    } else if (port == 2U) {
        RCC->AHBENR |= (1UL << 19);
    } else if (port == 3U) {
        RCC->AHBENR |= (1UL << 20);
    } else if (port == 5U) {
        RCC->AHBENR |= (1UL << 22);
    } else {
        /* ignore */
    }
}

static AegisErrorCode validate_pin(uint8_t port, uint8_t pin) {
    if (gpio_from_port(port) == NULL) {
        return ERR_INVALID_PARAM;
    }
    if (pin >= 16U) {
        return ERR_INVALID_PARAM;
    }
    return ERR_OK;
}

static uint32_t moder_bits_from_mode(AegisHalGpioMode mode) {
    /* 00=input, 01=output, 10=AF, 11=analog */
    if (mode == HAL_GPIO_MODE_INPUT) {
        return 0UL;
    }
    if (mode == HAL_GPIO_MODE_OUTPUT) {
        return 1UL;
    }
    if (mode == HAL_GPIO_MODE_AF) {
        return 2UL;
    }
    return 3UL;
}

static uint32_t pupdr_bits_from_pull(AegisHalGpioPull pull) {
    /* 00=none, 01=pull-up, 10=pull-down */
    if (pull == HAL_GPIO_PULL_NONE) {
        return 0UL;
    }
    if (pull == HAL_GPIO_PULL_UP) {
        return 1UL;
    }
    return 2UL;
}

/* ==================== 公共接口实现 ==================== */
AegisErrorCode aegis_hal_gpio_init(const AegisHalGpioConfig* config) {
    Stm32GpioRegs* gpio;
    uint32_t shift;
    uint32_t moder_bits;
    uint32_t pupdr_bits;
    AegisErrorCode ret;

    if (config == NULL) {
        return ERR_NULL_PTR;
    }

    ret = validate_pin(config->port, config->pin);
    if (ret != ERR_OK) {
        return ret;
    }

    gpio = gpio_from_port(config->port);
    if (gpio == NULL) {
        return ERR_INVALID_PARAM;
    }

    enable_gpio_clock(config->port);

    shift = (uint32_t)config->pin * 2UL;
    moder_bits = moder_bits_from_mode(config->mode);
    pupdr_bits = pupdr_bits_from_pull(config->pull);

    ENTER_CRITICAL();

    gpio->MODER &= ~(3UL << shift);
    gpio->MODER |= (moder_bits << shift);

    gpio->PUPDR &= ~(3UL << shift);
    gpio->PUPDR |= (pupdr_bits << shift);

    EXIT_CRITICAL();

    return ERR_OK;
}

AegisErrorCode aegis_hal_gpio_write(uint8_t port, uint8_t pin, AegisHalGpioLevel level) {
    Stm32GpioRegs* gpio;
    AegisErrorCode ret;

    ret = validate_pin(port, pin);
    if (ret != ERR_OK) {
        return ret;
    }

    gpio = gpio_from_port(port);
    if (gpio == NULL) {
        return ERR_INVALID_PARAM;
    }

    if (level == HAL_GPIO_LEVEL_HIGH) {
        gpio->BSRR = (1UL << pin);
    } else {
        gpio->BSRR = (1UL << (pin + 16U));
    }

    return ERR_OK;
}

AegisErrorCode aegis_hal_gpio_read(uint8_t port, uint8_t pin, AegisHalGpioLevel* level) {
    Stm32GpioRegs* gpio;
    AegisErrorCode ret;
    uint32_t v;

    if (level == NULL) {
        return ERR_NULL_PTR;
    }

    ret = validate_pin(port, pin);
    if (ret != ERR_OK) {
        return ret;
    }

    gpio = gpio_from_port(port);
    if (gpio == NULL) {
        return ERR_INVALID_PARAM;
    }

    v = (gpio->IDR >> pin) & 1UL;
    *level = (v != 0UL) ? HAL_GPIO_LEVEL_HIGH : HAL_GPIO_LEVEL_LOW;
    return ERR_OK;
}

AegisErrorCode aegis_hal_gpio_toggle(uint8_t port, uint8_t pin) {
    Stm32GpioRegs* gpio;
    AegisErrorCode ret;
    uint32_t v;

    ret = validate_pin(port, pin);
    if (ret != ERR_OK) {
        return ret;
    }

    gpio = gpio_from_port(port);
    if (gpio == NULL) {
        return ERR_INVALID_PARAM;
    }

    v = (gpio->ODR >> pin) & 1UL;
    if (v != 0UL) {
        gpio->BSRR = (1UL << (pin + 16U));
    } else {
        gpio->BSRR = (1UL << pin);
    }

    return ERR_OK;
}

