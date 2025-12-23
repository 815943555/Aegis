# application 模块
# 作者: jack liu

add_library(application OBJECT
    app_command.c
    app_cmd_service.c
    app_query.c
    app_init.c
)

target_include_directories(application PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../../include/common
    ${CMAKE_CURRENT_LIST_DIR}/../../include/infrastructure
    ${CMAKE_CURRENT_LIST_DIR}/../../include/domain
    ${CMAKE_CURRENT_LIST_DIR}/../../include/application
)
