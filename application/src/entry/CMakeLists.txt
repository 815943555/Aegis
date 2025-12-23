# entry 模块
# 作者: jack liu

add_library(entry OBJECT
    entry_init.c
    entry_main.c
)

target_include_directories(entry PUBLIC
    ${CMAKE_SOURCE_DIR}/include/common
    ${CMAKE_SOURCE_DIR}/include/hal
    ${CMAKE_SOURCE_DIR}/include/domain
    ${CMAKE_SOURCE_DIR}/include/application
    ${CMAKE_SOURCE_DIR}/include/entry
)
