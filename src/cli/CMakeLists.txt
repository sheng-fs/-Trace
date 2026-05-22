# ============================================================================
# src/cli - 命令行工具 trace
# ============================================================================

add_executable(trace_cli
    main.cpp
    commands/status.cpp
    commands/log.cpp
    commands/stat.cpp
    commands/watch.cpp
    commands/config.cpp
    commands/service.cpp
    commands/recover.cpp
    commands/export.cpp
)

target_link_libraries(trace_cli PRIVATE
    trace_core
    trace_common
    Qt6::Core
    Qt6::Sql
)