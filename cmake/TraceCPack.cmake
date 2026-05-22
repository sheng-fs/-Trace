# ============================================================================
# TraceCPack.cmake - CPack 打包配置（跨平台安装包生成）
# ============================================================================
include_guard(GLOBAL)

include(CPack)
include(GNUInstallDirs)

# ---------------------------------------------------------------------------
# 1. 通用 CPack 配置
# ---------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME                 "Trace")
set(CPACK_PACKAGE_VENDOR              "Trace Authors")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY  "跨平台系统活动与文件变动记录器")
set(CPACK_PACKAGE_VERSION_MAJOR       "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR       "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH       "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_HOMEPAGE_URL         "${PROJECT_HOMEPAGE_URL}")
set(CPACK_RESOURCE_FILE_LICENSE       "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README        "${CMAKE_SOURCE_DIR}/README.md")

set(CPACK_PACKAGE_INSTALL_DIRECTORY    "Trace")

# ---------------------------------------------------------------------------
# 2. 安装规则
# ---------------------------------------------------------------------------
install(TARGETS trace_core
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(TARGETS traced trace_gui trace_cli
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 配置文件
install(FILES
    config/trace.conf
    config/exclude_list.txt
    DESTINATION ${CMAKE_INSTALL_DATADIR}/trace/config
)

# 文档
install(DIRECTORY docs/
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    FILES_MATCHING PATTERN "*.md"
)

# ---------------------------------------------------------------------------
# 3. 平台特定打包
# ---------------------------------------------------------------------------
if(WIN32)
    # Windows: NSIS 安装器
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME          "迹 (Trace)")
    set(CPACK_NSIS_PACKAGE_NAME          "Trace")
    set(CPACK_NSIS_INSTALL_ROOT          "$PROGRAMFILES64")
    set(CPACK_NSIS_MUI_ICON              "${CMAKE_SOURCE_DIR}/src/gui/resources/trace.ico")
    set(CPACK_NSIS_MUI_UNIICON           "${CMAKE_SOURCE_DIR}/src/gui/resources/trace.ico")
    set(CPACK_NSIS_CREATE_ICONS_EXTRA    "")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA    "")
    set(CPACK_NSIS_URL_INFO_ABOUT        "${PROJECT_HOMEPAGE_URL}")
    set(CPACK_NSIS_HELP_LINK             "${PROJECT_HOMEPAGE_URL}")
    set(CPACK_NSIS_ENABLE_UNINSTALL_RENAME ON)

elseif(APPLE)
    # macOS: DMG + PKG
    set(CPACK_GENERATOR "DragNDrop;productbuild")
    set(CPACK_DMG_VOLUME_NAME            "Trace")
    set(CPACK_DMG_FORMAT                 "UDBZ")
    set(CPACK_BUNDLE_PLIST               "${CMAKE_SOURCE_DIR}/packaging/macos/Info.plist")

elseif(UNIX)
    # Linux: DEB + RPM
    set(CPACK_GENERATOR "DEB;RPM;TGZ")

    # DEB
    set(CPACK_DEBIAN_PACKAGE_SECTION     "utils")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS     "libc6 (>= 2.28), libstdc++6 (>= 9), libqt6widgets6 (>= 6.5)")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER  "Trace Authors")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE    "${PROJECT_HOMEPAGE_URL}")

    # RPM
    set(CPACK_RPM_PACKAGE_LICENSE        "MIT")
    set(CPACK_RPM_PACKAGE_REQUIRES       "glibc >= 2.28, libstdc++ >= 9, qt6-qtbase >= 6.5")
    set(CPACK_RPM_PACKAGE_URL            "${PROJECT_HOMEPAGE_URL}")
endif()

include(CPack)