# C89 DDD+CQRS 框架库
# 作者: jack liu

cmake_minimum_required(VERSION 3.10)
project(c_ddd_framework C)

# ==================== 编译器配置 ====================
# 强制 C89 标准
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 编译参数：全警告、警告视为错误、禁用扩展
add_compile_options(
    -std=c89
    -Wall
    -Wextra
    -Werror
    -pedantic
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wshadow
    -Wconversion
)

# ==================== 资源配置 ====================
# 内存池配置（单一内存块，多种块大小）
if(NOT DEFINED MEM_POOL_SMALL_SIZE)
    set(MEM_POOL_SMALL_SIZE 32)
endif()
if(NOT DEFINED MEM_POOL_SMALL_COUNT)
    set(MEM_POOL_SMALL_COUNT 16)
endif()

if(NOT DEFINED MEM_POOL_MEDIUM_SIZE)
    set(MEM_POOL_MEDIUM_SIZE 64)
endif()
if(NOT DEFINED MEM_POOL_MEDIUM_COUNT)
    set(MEM_POOL_MEDIUM_COUNT 8)
endif()

if(NOT DEFINED MEM_POOL_LARGE_SIZE)
    set(MEM_POOL_LARGE_SIZE 128)
endif()
if(NOT DEFINED MEM_POOL_LARGE_COUNT)
    set(MEM_POOL_LARGE_COUNT 4)
endif()

if(NOT DEFINED MEM_POOL_XLARGE_SIZE)
    set(MEM_POOL_XLARGE_SIZE 256)
endif()
if(NOT DEFINED MEM_POOL_XLARGE_COUNT)
    set(MEM_POOL_XLARGE_COUNT 2)
endif()

# 命令队列配置
if(NOT DEFINED CMD_QUEUE_SIZE)
    set(CMD_QUEUE_SIZE 16)
endif()

# 追溯日志配置
if(NOT DEFINED TRACE_LOG_SIZE)
    set(TRACE_LOG_SIZE 32)
endif()

# 领域事件配置
if(NOT DEFINED DOMAIN_EVENT_QUEUE_SIZE)
    set(DOMAIN_EVENT_QUEUE_SIZE 32)
endif()

if(NOT DEFINED DOMAIN_EVENT_HISTORY_SIZE)
    set(DOMAIN_EVENT_HISTORY_SIZE 16)
endif()

# 将配置传递给编译器
add_compile_definitions(
    MEM_POOL_SMALL_SIZE=${MEM_POOL_SMALL_SIZE}
    MEM_POOL_SMALL_COUNT=${MEM_POOL_SMALL_COUNT}
    MEM_POOL_MEDIUM_SIZE=${MEM_POOL_MEDIUM_SIZE}
    MEM_POOL_MEDIUM_COUNT=${MEM_POOL_MEDIUM_COUNT}
    MEM_POOL_LARGE_SIZE=${MEM_POOL_LARGE_SIZE}
    MEM_POOL_LARGE_COUNT=${MEM_POOL_LARGE_COUNT}
    MEM_POOL_XLARGE_SIZE=${MEM_POOL_XLARGE_SIZE}
    MEM_POOL_XLARGE_COUNT=${MEM_POOL_XLARGE_COUNT}
    CMD_QUEUE_SIZE=${CMD_QUEUE_SIZE}
    TRACE_LOG_SIZE=${TRACE_LOG_SIZE}
    DOMAIN_EVENT_QUEUE_SIZE=${DOMAIN_EVENT_QUEUE_SIZE}
    DOMAIN_EVENT_HISTORY_SIZE=${DOMAIN_EVENT_HISTORY_SIZE}
)

# ==================== 包含路径 ====================
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/common
    ${CMAKE_CURRENT_SOURCE_DIR}/include/infrastructure
    ${CMAKE_CURRENT_SOURCE_DIR}/include/domain
    ${CMAKE_CURRENT_SOURCE_DIR}/include/application
)

# ==================== 框架静态库 ====================
# Common 层
add_library(framework_common OBJECT
    src/common/error_codes.c
    src/common/mem_pool.c
    src/common/ring_buffer.c
    src/common/trace_log.c
)

# Domain 层
add_library(framework_domain OBJECT
    src/domain/domain_entity.c
    src/domain/domain_aggregate_root.c
    src/domain/domain_aggregate.c
    src/domain/domain_event.c
    src/domain/domain_value_object.c
    src/domain/domain_service.c
)

# Infrastructure 层（实现/适配）
add_library(framework_infrastructure OBJECT
    src/infrastructure/infrastructure_repository_inmem.c
)

# Application 层
add_library(framework_application OBJECT
    src/application/app_command.c
    src/application/app_cmd_service.c
    src/application/app_dto.c
    src/application/app_domain_assembler.c
    src/application/app_domain_converter.c
    src/application/app_module.c
    src/application/app_query.c
    src/application/app_init.c
)

# 组合为框架静态库
add_library(c_ddd_framework STATIC
    $<TARGET_OBJECTS:framework_common>
    $<TARGET_OBJECTS:framework_domain>
    $<TARGET_OBJECTS:framework_infrastructure>
    $<TARGET_OBJECTS:framework_application>
)

target_include_directories(c_ddd_framework PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/common>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/infrastructure>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/domain>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/application>
    $<INSTALL_INTERFACE:include/c_ddd_framework>
)

# ==================== 安装配置 ====================
# 安装头文件
install(DIRECTORY include/
    DESTINATION include/c_ddd_framework
    FILES_MATCHING PATTERN "*.h"
)

# 安装库文件
install(TARGETS c_ddd_framework
    EXPORT c_ddd_framework_targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# ==================== 导出配置 ====================
# 创建配置文件，方便其他项目使用
install(EXPORT c_ddd_framework_targets
    FILE c_ddd_framework-config.cmake
    DESTINATION lib/cmake/c_ddd_framework
)

message(STATUS "C DDD Framework 配置完成")
message(STATUS "  - 内存池: ${MEM_POOL_SMALL_COUNT}x${MEM_POOL_SMALL_SIZE}B + ${MEM_POOL_MEDIUM_COUNT}x${MEM_POOL_MEDIUM_SIZE}B + ${MEM_POOL_LARGE_COUNT}x${MEM_POOL_LARGE_SIZE}B + ${MEM_POOL_XLARGE_COUNT}x${MEM_POOL_XLARGE_SIZE}B")
message(STATUS "  - 命令队列大小: ${CMD_QUEUE_SIZE}")
message(STATUS "  - 事件队列大小: ${DOMAIN_EVENT_QUEUE_SIZE}")
message(STATUS "  - 事件历史大小: ${DOMAIN_EVENT_HISTORY_SIZE}")
