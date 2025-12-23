# minimal_app - 最小示例应用
# 作者: jack liu

cmake_minimum_required(VERSION 3.10)
project(minimal_app C)

# ==================== 编译器配置 ====================
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_compile_options(
    -std=c89
    -Wall
    -Wextra
    -pedantic
)

# ==================== 框架路径 ====================
set(FRAMEWORK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../framework)

# 包含框架头文件
include_directories(
    ${FRAMEWORK_DIR}/include/common
    ${FRAMEWORK_DIR}/include/domain
    ${FRAMEWORK_DIR}/include/application
    ${FRAMEWORK_DIR}/include/entry
)

# ==================== 链接框架库 ====================
# 如果框架未编译，先编译框架
if(NOT TARGET c_ddd_framework)
    add_subdirectory(${FRAMEWORK_DIR} ${CMAKE_BINARY_DIR}/framework)
endif()

# ==================== 平台移植层 ====================
set(TARGET_PLATFORM "x86_sim")

add_library(minimal_app_port STATIC
    ${FRAMEWORK_DIR}/port/${TARGET_PLATFORM}/port_critical.c
    ${FRAMEWORK_DIR}/port/${TARGET_PLATFORM}/port_hal_gpio.c
    ${FRAMEWORK_DIR}/port/${TARGET_PLATFORM}/port_hal_timer.c
    ${FRAMEWORK_DIR}/port/${TARGET_PLATFORM}/entry_platform.c
)
target_include_directories(minimal_app_port PRIVATE
    ${FRAMEWORK_DIR}/include/common
    ${FRAMEWORK_DIR}/include/domain
    ${FRAMEWORK_DIR}/include/infrastructure
)

# ==================== 示例应用 ====================
add_executable(minimal_app
    main.c
    demo_domain.c
    demo_application.c
    ${FRAMEWORK_DIR}/src/entry/entry_init.c
    ${FRAMEWORK_DIR}/src/entry/entry_main.c
)

target_link_libraries(minimal_app
    minimal_app_port
    c_ddd_framework
)

message(STATUS "minimal_app 配置完成")
