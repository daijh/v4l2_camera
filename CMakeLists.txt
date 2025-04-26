cmake_minimum_required(VERSION 3.16)
project(v4l2_camera)

include(config/compiler.cmake)
include(config/format.cmake)

include(FetchContent)

# PkgConfig
find_package(PkgConfig REQUIRED)

# libyuv

# libdrm
pkg_check_modules(libdrm REQUIRED IMPORTED_TARGET "libdrm")

# SDL2
find_package(SDL2 REQUIRED)

# cxxopts
FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG v3.2.1
  EXCLUDE_FROM_ALL)
FetchContent_MakeAvailable(cxxopts)
include_directories("${cxxopts_SOURCE_DIR}/include")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out")

add_subdirectory(src)
