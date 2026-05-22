# ============================================================================
# src/uninstaller - 卸载程序 TraceUninstaller
# ============================================================================

add_executable(trace_uninstaller main.cpp)

target_link_libraries(trace_uninstaller PRIVATE trace_common Qt6::Core)

if(WIN32)
    set_target_properties(trace_uninstaller PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    set_target_properties(trace_uninstaller PROPERTIES MACOSX_BUNDLE TRUE)
endif()