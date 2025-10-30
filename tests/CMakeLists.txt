add_executable(test writetest.cpp)
target_link_libraries(test
    hpSerialRead
    CppLinuxSerial
)