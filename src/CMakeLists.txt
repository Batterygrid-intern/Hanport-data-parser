add_library(hanportMessageValidator STATIC HanportMessageValidator.cpp)
target_include_directories(hanportMessageValidator 
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

add_executable(${PROJECT_NAME} main.cpp)
add_compile_definitions(EX_DATA_PATH=\"$ENV{ex_data}\")
target_link_libraries(${PROJECT_NAME}
    hanport_data
)

