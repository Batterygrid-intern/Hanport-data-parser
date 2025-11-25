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

#Build modbus server lib
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
#Build mqttpub lib
add_library(HpMqttPub STATIC hpMqttPub.cpp)
target_include_directories(HpMqttPub
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(HpMqttPub
        PUBLIC
        PahoMqttCpp::paho-mqttpp3
)

#Config
add_library(config STATIC config.cpp)
target_include_directories(config
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
add_library(hpLogger STATIC hpLogger.cpp)
target_include_directories(hpLogger
    PUBLIC    
    ${CMAKE_SOURCE_DIR}/include
)

# Let the source know libmodbus was found (so code can #ifdef LIBMODBUS_FOUND)
target_compile_definitions(hpModbuss PUBLIC LIBMODBUS_FOUND)
# build executable and link to libraries
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME}
    hpMessageValidator
    hpDataParser 
    hpData
    hpSerialRead
    hpModbuss
    config
    HpMqttPub
    spdlog::spdlog_header_only
)
