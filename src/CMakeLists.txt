# build messagevalidator library
add_library(hpMessageValidator STATIC hpMessageValidator.cpp)
target_include_directories(hpMessageValidator 
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)
# build hpDataParser
add_library(hpDataParser STATIC hpDataParser.cpp)
target_include_directories(hpDataParser
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)
#build serial read lib
add_library(hpSerialRead STATIC hpSerialRead.cpp)
target_include_directories(hpSerialRead
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

add_library(filereader STATIC filereader.cpp)
target_include_directories(filereader
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

add_library(hpModbuss STATIC hpModbuss.cpp)
target_include_directories(hpModbuss
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${LIBMODBUS_INCLUDE_DIRS}
)
target_link_libraries(hpModbuss
    PUBLIC 
     ${LIBMODBUS_LIBRARIES}
)
add_library(HpMqttPub STATIC hpMqttPub.cpp Config.cpp)
target_include_directories(HpMqttPub
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(HpMqttPub
        PUBLIC
        PahoMqttCpp::paho-mqttpp3

)

# Let the source know libmodbus was found (so code can #ifdef LIBMODBUS_FOUND)
target_compile_definitions(hpModbuss PUBLIC LIBMODBUS_FOUND)
# build executable and link to libraries
add_executable(${PROJECT_NAME} main.cpp)
add_compile_definitions(EX_DATA_PATH=\"$ENV{ex_data}\")
target_link_libraries(${PROJECT_NAME}
    hpMessageValidator
    hpDataParser 
    hpData
    hpSerialRead
    hpModbuss
    HpMqttPub
)

#Test and example files can be removed in production
add_executable(writetest writetest.cpp)
target_link_libraries(writetest
    CppLinuxSerial 
    filereader
   
)



add_executable(modbus_example modbus_example.cpp)
target_link_libraries(modbus_example
    hpModbuss
    ${LIBMODBUS_LIBRARIES}
)
target_include_directories(modbus_example PUBLIC ${CMAKE_SOURCE_DIR}/include ${LIBMODBUS_INCLUDE_DIRS})

# integration test: parse sample message, start server and read back registers with libmodbus
add_executable(modbus_integration_test ../tests/modbus_integration_test.cpp)
target_include_directories(modbus_integration_test PUBLIC ${CMAKE_SOURCE_DIR}/include ${LIBMODBUS_INCLUDE_DIRS})
target_link_libraries(modbus_integration_test
    PRIVATE
    gtest_main
    hpModbuss
    hpMessageValidator
    hpDataParser
    ${LIBMODBUS_LIBRARIES}
)
add_test(NAME modbus_integration_test COMMAND modbus_integration_test)

# optional C++ client to read remote Modbus registers
add_executable(modbus_client modbus_client.cpp)
target_include_directories(modbus_client PUBLIC ${CMAKE_SOURCE_DIR}/include ${LIBMODBUS_INCLUDE_DIRS})
target_link_libraries(modbus_client PRIVATE ${LIBMODBUS_LIBRARIES})


#add_executable(readtest readtest.cpp)
#target_link_libraries(readtest
#    CppLinuxSerial
#)

