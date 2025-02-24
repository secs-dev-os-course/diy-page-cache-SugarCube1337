cmake_minimum_required(VERSION 3.12)
project(Cache
        VERSION 0.0.0
        DESCRIPTION "Cache library"
        LANGUAGES CXX
)

add_library(Cache SHARED
        CacheAPI.cpp
        CacheAPI.h
        CacheManager.cpp
        CacheManager.h
)

target_include_directories(Cache PUBLIC ${CMAKE_SOURCE_DIR})

target_compile_definitions(Cache PRIVATE CACHE_EXPORTS)

set(COPY_TO_APP ON)
if (COPY_TO_APP)
    add_custom_command(TARGET Cache POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Cache>"
            "${CMAKE_BINARY_DIR}/app"
    )
endif()