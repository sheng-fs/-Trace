# ============================================================================
# src/gui - 图形界面程序 Trace
# ============================================================================

add_executable(trace_gui
    main.cpp
    mainwindow.h
    mainwindow.cpp
    mainwindow.ui
    dailyview.cpp
    eventlist.cpp
    settingsdialog.cpp
    recoverytool.cpp
    trayicon.cpp
    resources/trace.qrc
)

# 当作为 macOS bundle 时，设置图标
if(APPLE)
    set_target_properties(trace_gui PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/packaging/macos/Info.plist"
        MACOSX_BUNDLE_ICON_FILE "trace.icns"
        WIN32_EXECUTABLE TRUE
    )
elseif(WIN32)
    set_target_properties(trace_gui PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

target_link_libraries(trace_gui PRIVATE
    trace_core
    trace_common
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
)

# Qt Charts（可选）
if(TARGET Qt6::Charts)
    target_link_libraries(trace_gui PRIVATE Qt6::Charts)
    target_compile_definitions(trace_gui PRIVATE TRACE_HAS_CHARTS)
endif()