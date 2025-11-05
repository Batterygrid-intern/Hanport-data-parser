cmake_minimum_required(VERSION 3.16)
project(test LANGUAGES C CXX)
find_package(PkgConfig REQUIRED)
find_package(PahoMqttCpp REQUIRED)
pkg_check_modules(PAHO_MQTT_CPP REQUIRED paho-mqttpp3)
pkg_check_modules(LIBMODBUS REQUIRED libmodbus)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")

include(FetchContent)
FetchContent_Declare(
    CppLinuxSerial
    GIT_REPOSITORY https://github.com/gbmhunter/CppLinuxSerial.git
    GIT_TAG master
)
FetchContent_MakeAvailable(CppLinuxSerial)

add_subdirectory(include)
add_subdirectory(src)
