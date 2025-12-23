# 示例应用 - 使用 C DDD 框架
# 作者: jack liu

cmake_minimum_required(VERSION 3.10)
project(c_ddd_demo_app C)

# ==================== 编译器配置 ====================
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_compile_options(
    -std=c89
    -Wall
    -Wextra
    -Werror
    -pedantic
)

# ==================== 平台配置 ====================
if(NOT DEFINED TARGET_PLATFORM)
    set(TARGET_PLATFORM "x86_sim")
endif()
message(STATUS "应用目标平台: ${TARGET_PLATFORM}")

# ==================== 链接框架库 ====================
# 框架库路径
set(FRAMEWORK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../framework)

# 包含框架头文件
include_directories(
    ${FRAMEWORK_DIR}/include/common
    ${FRAMEWORK_DIR}/include/domain
    ${FRAMEWORK_DIR}/include/application
    ${CMAKE_CURRENT_SOURCE_DIR}/include/entry
)

# 链接框架库（假设已经编译）
# 如果框架未编译，需要先编译框架
if(NOT TARGET c_ddd_framework)
    add_subdirectory(${FRAMEWORK_DIR} ${CMAKE_BINARY_DIR}/framework)
endif()

# ==================== 平台移植层 ====================
add_library(app_port STATIC
    ${FRAMEWORK_DIR}/port/${TARGET_PLATFORM}/port_critical.c
    ${FRAMEWORK_DIR}/port/${TARGET_PLATFORM}/port_hal_gpio.c
    ${FRAMEWORK_DIR}/port/${TARGET_PLATFORM}/port_hal_timer.c
    ${FRAMEWORK_DIR}/port/${TARGET_PLATFORM}/entry_platform.c
)
target_include_directories(app_port PRIVATE
    ${FRAMEWORK_DIR}/include/common
    ${FRAMEWORK_DIR}/include/domain
    ${FRAMEWORK_DIR}/include/infrastructure
)

# ==================== 应用层代码 ====================
add_executable(c_ddd_demo_app
    src/main.c
    src/entry/entry_init.c
    src/entry/entry_main.c
)

target_link_libraries(c_ddd_demo_app
    app_port
    c_ddd_framework
)

message(STATUS "示例应用配置完成")
