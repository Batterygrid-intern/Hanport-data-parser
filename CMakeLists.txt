cmake_minimum_required(VERSION 3.16)
project(test LANGUAGES C CXX)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_C_STANDRAD 11)
set(CMAKE_CXX_STANDRAD 17)

add_subdirectory(src)
