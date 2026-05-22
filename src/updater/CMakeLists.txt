# ============================================================================
# src/updater - 在线更新程序 TraceUpdater
# ============================================================================

add_executable(trace_updater main.cpp)

target_link_libraries(trace_updater PRIVATE trace_common Qt6::Core Qt6::Network)

if(TRACE_USE_LIBCURL AND CURL_FOUND)
    target_link_libraries(trace_updater PRIVATE CURL::libcurl)
    target_compile_definitions(trace_updater PRIVATE TRACE_HAS_CURL)
endif()