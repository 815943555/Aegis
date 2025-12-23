# 测试模块 CMakeLists.txt
# 作者: jack liu

# ==================== 端口层（critical section） ====================
set(FRAMEWORK_DIR ${CMAKE_SOURCE_DIR}/framework)

add_library(tests_port STATIC
    ${FRAMEWORK_DIR}/port/${TARGET_PLATFORM}/port_critical.c
)
target_include_directories(tests_port PRIVATE
    ${FRAMEWORK_DIR}/include/common
)

# ==================== 内存池测试 ====================
add_executable(test_mem_pool
    common/test_mem_pool.c
)
target_link_libraries(test_mem_pool c_ddd_framework tests_port)
add_test(NAME mem_pool_test COMMAND test_mem_pool)

# ==================== 应用层命令测试 ====================
add_executable(test_app_command
    application/test_app_command.c
)
target_link_libraries(test_app_command c_ddd_framework tests_port)
add_test(NAME app_command_test COMMAND test_app_command)

# ==================== 领域事件总线测试 ====================
add_executable(test_domain_event
    domain/test_domain_event.c
)
target_link_libraries(test_domain_event c_ddd_framework tests_port)
add_test(NAME domain_event_test COMMAND test_domain_event)

# ==================== 领域事件总线边界测试 ====================
add_executable(test_domain_event_edge_cases
    domain/test_domain_event_edge_cases.c
)
target_link_libraries(test_domain_event_edge_cases c_ddd_framework tests_port)
add_test(NAME domain_event_edge_cases_test COMMAND test_domain_event_edge_cases)

# ==================== 仓储事件集成测试 ====================
add_executable(test_repository_event_integration
    integration/test_repository_event_integration.c
)
target_link_libraries(test_repository_event_integration c_ddd_framework tests_port)
add_test(NAME repository_event_integration_test COMMAND test_repository_event_integration)

# ==================== 测试报告 ====================
# 添加自定义目标运行所有测试
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS test_mem_pool test_app_command test_domain_event test_domain_event_edge_cases test_repository_event_integration
    COMMENT "运行所有单元测试..."
)

# 如果启用代码覆盖率
if(ENABLE_COVERAGE)
    find_program(LCOV_EXECUTABLE lcov)
    find_program(GENHTML_EXECUTABLE genhtml)

    if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
        # 生成覆盖率报告
        add_custom_target(coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${LCOV_EXECUTABLE} --capture --directory ${CMAKE_BINARY_DIR}
                    --output-file coverage.info --rc lcov_branch_coverage=1
            COMMAND ${LCOV_EXECUTABLE} --remove coverage.info '/usr/*'
                    --output-file coverage.info --rc lcov_branch_coverage=1
            COMMAND ${GENHTML_EXECUTABLE} coverage.info --output-directory coverage_html
                    --rc lcov_branch_coverage=1
            DEPENDS test_mem_pool test_app_command test_domain_event test_domain_event_edge_cases test_repository_event_integration
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "生成代码覆盖率报告..."
        )

        message(STATUS "代码覆盖率报告将生成到: ${CMAKE_BINARY_DIR}/coverage_html")
    else()
        message(WARNING "未找到 lcov 或 genhtml,无法生成覆盖率报告")
    endif()
endif()
