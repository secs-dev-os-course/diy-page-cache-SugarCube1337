cmake_minimum_required(VERSION 3.12)
project(PageCache
        VERSION 0.0.0
        DESCRIPTION "PageCache project"
        LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(Cache)
add_subdirectory(app)