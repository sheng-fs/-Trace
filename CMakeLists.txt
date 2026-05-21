cmake_minimum_required(VERSION 3.15)
project(Trace VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(TraceDeps REQUIRED)

add_subdirectory(src/common)
add_subdirectory(src/core)
add_subdirectory(src/daemon)
add_subdirectory(src/gui)
add_subdirectory(src/updater)
add_subdirectory(src/uninstaller)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

include(TraceCPack)