# ============================================================================
# src/common - 公共组件库（IPC 客户端、版本信息、工具函数）
# ============================================================================

add_library(trace_common STATIC
    ipc_client.h
    ipc_client.cpp
    utils.h
    utils.cpp
)

target_include_directories(trace_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 版本头文件（由 CMake 从 version.h.in 生成）
configure_file(
    version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

target_include_directories(trace_common PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(trace_common PUBLIC Qt6::Core)