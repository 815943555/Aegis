# STM32F030 移植示例（Aegis port）

本目录提供 STM32F030（Cortex-M0）平台的移植示例代码，用于说明如何为 Aegis 补齐 `framework/include/infrastructure/hal_*.h` 与 `framework/include/entry/entry_platform.h` 的平台实现。

## 文件说明

- `port_critical.c`：临界区实现（PRIMASK + 嵌套计数）。
- `port_hal_gpio.c`：GPIO 寄存器级实现（RCC AHBENR + GPIOx MODER/PUPDR/IDR/ODR/BSRR）。
- `port_hal_timer.c`：基于 SysTick 的 `aegis_hal_timer_get_tick_ms()` 示例（COUNTFLAG 轮询更新毫秒计数）；`init/start/stop` 提供软件定时器示例（精度按 ms）。
- `entry_platform.c`：平台侧依赖装配（默认使用 inmem 仓储实现，并以 now_ms 回调提供时间戳）。

## 如何接入（示例步骤）

1) 在你的 MCU 工程中选择该平台：

```bash
cmake -S application -B build/stm32f030 -DTARGET_PLATFORM=stm32f030
```

2) 由启动代码/系统初始化配置 SysTick 产生 1ms tick（建议开启 SysTick 中断）。

3) 若你启用 SysTick 中断，建议在 `SysTick_Handler` 中周期性调用一次 `aegis_hal_timer_get_tick_ms()`（用于更新内部 tick 计数；也可在主循环频繁调用）。

4) `Entry` 侧按现有方式初始化：

- 先 `aegis_entry_platform_init(now_ms_fn, ctx)` 获取写仓储接口
- 再把 `aegis_entry_platform_get_write_repo()` 注入 `aegis_entry_init_all()`

## 注意

- 本目录为“移植示例/参考实现”，并未包含启动文件、链接脚本、时钟树初始化等工程化内容。
- 若你使用 STM32 官方 CMSIS/HAL，建议把寄存器访问替换为对应库接口，但保持 Aegis 的分层与 DI 约束不变。

