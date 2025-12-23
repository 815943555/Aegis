# domain 模块
# 作者: jack liu

add_library(domain OBJECT
    domain_entity.c
    domain_aggregate_root.c
    domain_repository.c
    domain_event.c
)

target_include_directories(domain PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../../include/common
    ${CMAKE_CURRENT_LIST_DIR}/../../include/domain
    ${CMAKE_CURRENT_LIST_DIR}/../../include/infrastructure
)
