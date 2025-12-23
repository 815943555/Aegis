# common 模块
# 作者: jack liu

add_library(common OBJECT
    mem_pool.c
    trace_log.c
    error_codes.c
    ring_buffer.c
)

target_include_directories(common PUBLIC
    ${CMAKE_SOURCE_DIR}/include/common
)
