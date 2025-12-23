# Infrastructure 模块（头文件 + 平台 port 实现）
# 作者: jack liu

add_library(framework_infrastructure INTERFACE)

target_include_directories(framework_infrastructure INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/../../include/common
    ${CMAKE_CURRENT_LIST_DIR}/../../include/infrastructure
)
