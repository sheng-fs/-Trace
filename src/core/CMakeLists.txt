# ============================================================================
# src/core - 核心监控库 libtrace_core（静态库）
# ============================================================================

add_library(trace_core STATIC
    # 文件监控
    watcher/file_watcher.h
    watcher/file_watcher_win.cpp
    watcher/file_watcher_mac.mm
    watcher/file_watcher_linux.cpp

    # 硬件监控
    hardware/hardware_monitor.h
    hardware/hw_monitor_win.cpp
    hardware/hw_monitor_mac.mm
    hardware/hw_monitor_linux.cpp

    # 系统监控
    system/system_monitor.h
    system/sys_monitor_win.cpp
    system/sys_monitor_mac.mm
    system/sys_monitor_linux.cpp

    # 事件模型
    event.h

    # 日志记录
    logger.h
    logger.cpp

    # 配置管理
    config.h
    config.cpp

    # 恢复辅助
    recovery.h
    recovery.cpp
)

target_include_directories(trace_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(trace_core PUBLIC Qt6::Core Qt6::Sql trace_platform)

# 平台特定源文件过滤
target_sources(trace_core PRIVATE
    $<$<PLATFORM_ID:Windows>: watcher/file_watcher_win.cpp hardware/hw_monitor_win.cpp system/sys_monitor_win.cpp>
    $<$<PLATFORM_ID:Darwin>:  watcher/file_watcher_mac.mm  hardware/hw_monitor_mac.mm  system/sys_monitor_mac.mm>
    $<$<PLATFORM_ID:Linux>:  watcher/file_watcher_linux.cpp hardware/hw_monitor_linux.cpp system/sys_monitor_linux.cpp>
)