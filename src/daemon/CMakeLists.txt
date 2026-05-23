# ============================================================================
# src/daemon - 守护进程 traced（后台服务，需高权限运行）
# ============================================================================

add_executable(traced
    main.cpp
    service_manager.h
    ipc_server.h
    ipc_server.cpp
)

target_sources(traced PRIVATE
    $<$<PLATFORM_ID:Windows>: service_win.cpp>
    $<$<PLATFORM_ID:Darwin>:  service_mac.mm>
    $<$<PLATFORM_ID:Linux>:  service_linux.cpp>
)

target_link_libraries(traced PRIVATE trace_core trace_common trace_platform Qt6::Core Qt6::Network)