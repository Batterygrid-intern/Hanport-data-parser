add_library(hanport_data STATIC hanport_data.cpp)
target_include_directories(hanport_data 
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

add_executable(${PROJECT_NAME} main.cpp)
add_compile_definitions(EX_DATA_PATH=\"$ENV{ex_data}\")
target_link_libraries(${PROJECT_NAME}
    hanport_data
)

