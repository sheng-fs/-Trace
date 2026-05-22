# ============================================================================
# tests - 单元测试与集成测试
# ============================================================================

# ---- 单元测试 ----

add_executable(test_watcher
    unit/test_watcher.cpp
)
target_link_libraries(test_watcher PRIVATE trace_core GTest::gtest GTest::gtest_main)
gtest_discover_tests(test_watcher)

add_executable(test_logger
    unit/test_logger.cpp
)
target_link_libraries(test_logger PRIVATE trace_core GTest::gtest GTest::gtest_main)
gtest_discover_tests(test_logger)

add_executable(test_config
    unit/test_config.cpp
)
target_link_libraries(test_config PRIVATE trace_core GTest::gtest GTest::gtest_main)
gtest_discover_tests(test_config)

add_executable(test_hw_monitor
    unit/test_hw_monitor.cpp
)
target_link_libraries(test_hw_monitor PRIVATE trace_core GTest::gtest GTest::gtest_main)
gtest_discover_tests(test_hw_monitor)

# ---- 集成测试 ----

add_executable(test_service_lifecycle
    integration/test_service_lifecycle.cpp
)
target_link_libraries(test_service_lifecycle PRIVATE trace_core trace_common GTest::gtest GTest::gtest_main)
gtest_discover_tests(test_service_lifecycle)

add_executable(test_ipc
    integration/test_ipc.cpp
)
target_link_libraries(test_ipc PRIVATE trace_core trace_common GTest::gtest GTest::gtest_main)
gtest_discover_tests(test_ipc)