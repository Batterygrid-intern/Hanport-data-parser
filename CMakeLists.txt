cmake_minimum_required(VERSION 3.16)
project(test LANGUAGES C CXX)
find_package(PkgConfig REQUIRED)
find_package(PahoMqttCpp REQUIRED)
pkg_check_modules(LIBMODBUS REQUIRED libmodbus)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")

add_subdirectory(include)
add_subdirectory(src)
