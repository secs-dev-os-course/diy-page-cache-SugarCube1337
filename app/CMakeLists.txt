cmake_minimum_required(VERSION 3.12)
project(app
        VERSION 0.0.0
        DESCRIPTION "Application module"
        LANGUAGES CXX
)

add_executable(app app.cpp)

target_include_directories(app PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(app PRIVATE Cache)