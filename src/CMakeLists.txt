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

# build executable and link to libraries
add_executable(${PROJECT_NAME} main.cpp)
add_compile_definitions(EX_DATA_PATH=\"$ENV{ex_data}\")
target_link_libraries(${PROJECT_NAME}
    hpMessageValidator
    hpDataParser 
    hpData
    hpSerialRead
)

