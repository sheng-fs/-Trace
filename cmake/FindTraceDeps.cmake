# ============================================================================
# FindTraceDeps.cmake - 查找并配置 Trace 项目所需所有依赖
# ============================================================================
include_guard(GLOBAL)

# ---------------------------------------------------------------------------
# 1. Qt 6 框架
# ---------------------------------------------------------------------------
set(TRACE_QT_COMPONENTS Core Widgets Sql Network)
option(BUILD_GUI_CHARTS "启用图表组件 (Qt Charts)" ON)

if(BUILD_GUI_CHARTS)
    list(APPEND TRACE_QT_COMPONENTS Charts)
endif()

find_package(Qt6 REQUIRED COMPONENTS ${TRACE_QT_COMPONENTS})

message(STATUS "Qt 版本: ${Qt6_VERSION}")
message(STATUS "Qt 安装路径: ${Qt6_DIR}")

# 自动启用 Qt 的 MOC/UIC/RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ---------------------------------------------------------------------------
# 2. SQLite（优先使用 Qt 内建 SQLite）
# ---------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Sql)

# Qt 6.2+ 自带 SQLite 插件，无需额外配置
# 低版本需自行查找：
find_package(SQLite3 QUIET)
if(SQLite3_FOUND)
    message(STATUS "使用系统 SQLite ${SQLite3_VERSION}")
    set(TRACE_USE_SYSTEM_SQLITE ON)
else()
    message(STATUS "使用 Qt 内建 SQLite")
    set(TRACE_USE_SYSTEM_SQLITE OFF)
endif()

# ---------------------------------------------------------------------------
# 3. Google Test（仅测试需要）
# ---------------------------------------------------------------------------
if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    FetchContent_MakeAvailable(googletest)
    message(STATUS "Google Test 已通过 FetchContent 引入")
endif()

# ---------------------------------------------------------------------------
# 4. libcurl（更新程序需要）
# ---------------------------------------------------------------------------
option(TRACE_USE_LIBCURL "使用 libcurl 进行在线更新" ON)
if(TRACE_USE_LIBCURL)
    find_package(CURL QUIET)
    if(CURL_FOUND)
        message(STATUS "libcurl 版本: ${CURL_VERSION}")
    else()
        message(WARNING "未找到 libcurl，更新功能将不可用。"
                        "请安装 libcurl 开发包。")
        set(TRACE_USE_LIBCURL OFF)
    endif()
endif()

# ---------------------------------------------------------------------------
# 5. 平台检测与系统库
#    trace_platform 是一个 INTERFACE 虚目标，承载平台原生库依赖，
#    后续 trace_core 和 traced 链接它即可。
# ---------------------------------------------------------------------------
add_library(trace_platform INTERFACE)

if(WIN32)
    set(TRACE_PLATFORM "Windows")
    add_compile_definitions(TRACE_PLATFORM_WINDOWS)

    target_link_libraries(trace_platform INTERFACE
        advapi32
        user32
        shell32
    )

elseif(APPLE)
    set(TRACE_PLATFORM "macOS")
    add_compile_definitions(TRACE_PLATFORM_MACOS)

    find_library(FOUNDATION_LIB Foundation REQUIRED)
    find_library(CORESERVICES_LIB CoreServices REQUIRED)
    find_library(IOKIT_LIB IOKit REQUIRED)
    find_library(APPKIT_LIB AppKit REQUIRED)

    target_link_libraries(trace_platform INTERFACE
        ${FOUNDATION_LIB}
        ${CORESERVICES_LIB}
        ${IOKIT_LIB}
        ${APPKIT_LIB}
    )

elseif(UNIX)
    set(TRACE_PLATFORM "Linux")
    add_compile_definitions(TRACE_PLATFORM_LINUX)

    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SYSTEMD libsystemd)
        if(SYSTEMD_FOUND)
            message(STATUS "找到 systemd ${SYSTEMD_VERSION}")
            target_link_libraries(trace_platform INTERFACE
                ${SYSTEMD_LIBRARIES})
            target_include_directories(trace_platform INTERFACE
                ${SYSTEMD_INCLUDE_DIRS})
        endif()

        pkg_check_modules(UDEV libudev)
        if(UDEV_FOUND)
            message(STATUS "找到 libudev ${UDEV_VERSION}")
            target_link_libraries(trace_platform INTERFACE
                ${UDEV_LIBRARIES})
            target_include_directories(trace_platform INTERFACE
                ${UDEV_INCLUDE_DIRS})
        endif()
    endif()

    target_link_libraries(trace_platform INTERFACE pthread)
endif()

message(STATUS "目标平台: ${TRACE_PLATFORM}")

# ---------------------------------------------------------------------------
# 6. 全局编译定义
# ---------------------------------------------------------------------------
add_compile_definitions(
    TRACE_VERSION="${PROJECT_VERSION}"
    TRACE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    TRACE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    TRACE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

