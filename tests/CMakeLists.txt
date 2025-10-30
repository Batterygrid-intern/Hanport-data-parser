add_library(hpSerialRead ../src/hpSerialRead.cpp)
target_include_directories(hpSerialRead
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)
add_executable(test2 writetest.cpp)

target_link_libraries(test
    hpSerialRead
    CppLinuxSerial
)