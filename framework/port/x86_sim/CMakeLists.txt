# x86_sim 平台适配层
# 作者: jack liu

add_library(port OBJECT
    port_critical.c
    port_hal_gpio.c
    port_hal_timer.c
)

target_include_directories(port PUBLIC
    ${CMAKE_SOURCE_DIR}/include/common
    ${CMAKE_SOURCE_DIR}/include/hal
)
