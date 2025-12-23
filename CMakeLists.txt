cmake_minimum_required(VERSION 3.10)
project(c_ddd_project C)

# ==================== 全局配置 ====================
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

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

# ==================== 平台配置 ====================
if(NOT DEFINED TARGET_PLATFORM)
    set(TARGET_PLATFORM "x86_sim")
    message(STATUS "No TARGET_PLATFORM specified, using default: x86_sim")
endif()
message(STATUS "Target platform: ${TARGET_PLATFORM}")

# ==================== 资源配置（可选） ====================
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

if(NOT DEFINED CMD_QUEUE_SIZE)
    set(CMD_QUEUE_SIZE 16)
endif()

if(NOT DEFINED TRACE_LOG_SIZE)
    set(TRACE_LOG_SIZE 32)
endif()

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
    TARGET_PLATFORM_${TARGET_PLATFORM}=1
)

# ==================== 构建选项 ====================
option(BUILD_FRAMEWORK "Build framework library" ON)
option(BUILD_APPLICATION "Build demo application" ON)
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_TESTS "Build unit tests" ON)

# ==================== 框架 / 应用 / 示例 / 测试 ====================
if(BUILD_FRAMEWORK)
    add_subdirectory(framework)
endif()

if(BUILD_APPLICATION)
    add_subdirectory(application)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples/minimal_app)
    add_subdirectory(examples/event_driven)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ==================== 约束检查 ====================
option(ENABLE_CONSTRAINT_CHECKS "Enable constraint checks" ON)

if(ENABLE_CONSTRAINT_CHECKS)
    find_package(Python3 COMPONENTS Interpreter)

    if(Python3_FOUND)
        add_custom_target(constraint_checks
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/check_naming.py
                    ${CMAKE_SOURCE_DIR}/framework
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/check_layering.py
                    ${CMAKE_SOURCE_DIR}/framework
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/check_no_globals.py
                    ${CMAKE_SOURCE_DIR}/framework
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/check_isr_safety.py
                    ${CMAKE_SOURCE_DIR}/framework
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/check_traceability.py
                    ${CMAKE_SOURCE_DIR}/framework
            COMMAND ${CMAKE_SOURCE_DIR}/tools/check_no_malloc.sh ${CMAKE_SOURCE_DIR}/framework
            COMMENT "运行架构约束检查（framework）..."
            VERBATIM
        )

        message(STATUS "约束检查已启用: 使用 'make constraint_checks' 运行")
    else()
        message(WARNING "未找到 Python3,无法运行约束检查")
    endif()
endif()
