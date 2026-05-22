cmake_minimum_required(VERSION 3.20)

# ============================================================================
# 项目基本信息
# ============================================================================
project(Trace
    VERSION 1.0.0
    DESCRIPTION "跨平台系统活动与文件变动记录器"
    HOMEPAGE_URL "https://github.com/sheng-fs/-Trace"
    LANGUAGES CXX
)

# ============================================================================
# 全局编译配置
# ============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# 生成 compile_commands.json（供 IDE 和 clang-tidy 使用）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# 编译选项
# ============================================================================
if(MSVC)
    add_compile_options(/W4 /utf-8 /permissive-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ============================================================================
# 构建类型默认值
# ============================================================================
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Build type: Debug, Release, RelWithDebInfo, MinSizeRel" FORCE)
endif()

# ============================================================================
# 模块开关
# ============================================================================
option(BUILD_TESTS    "构建测试"      ON)
option(BUILD_GUI      "构建 GUI 界面"  ON)
option(BUILD_CLI      "构建 CLI 工具"  ON)
option(BUILD_DAEMON   "构建守护进程"   ON)
option(BUILD_UPDATER  "构建更新程序"   ON)
option(BUILD_UNINSTALLER "构建卸载程序" ON)

# ============================================================================
# 依赖查找
# ============================================================================
find_package(TraceDeps REQUIRED)

# ============================================================================
# 子项目
# ============================================================================
add_subdirectory(src/common)

# 核心监控库（纯静态库，无 Qt 依赖，守护进程和 CLI 共用）
add_subdirectory(src/core)

# 守护进程 - 后台服务，需要平台原生 API
if(BUILD_DAEMON)
    add_subdirectory(src/daemon)
endif()

# Qt GUI 界面
if(BUILD_GUI)
    add_subdirectory(src/gui)
endif()

# CLI 命令行工具
if(BUILD_CLI)
    add_subdirectory(src/cli)
endif()

# 在线更新程序
if(BUILD_UPDATER)
    add_subdirectory(src/updater)
endif()

# 卸载程序
if(BUILD_UNINSTALLER)
    add_subdirectory(src/uninstaller)
endif()

# ============================================================================
# 测试
# ============================================================================
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ============================================================================
# CPack 打包
# ============================================================================
include(TraceCPack)