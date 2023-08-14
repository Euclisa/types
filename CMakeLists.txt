cmake_minimum_required(VERSION 3.0)
project(types)
set (CMAKE_CXX_STANDARD 20)
find_package(GTest REQUIRED)
file(GLOB CPP_SOURCES_MAIN
                main.cpp
            )
#[[
file(GLOB CPP_SOURCES_TEST
                tests/algo/fft.cpp
                src/algo/fft.cpp
            )

add_executable(TEST ${CPP_SOURCES_TEST})
target_include_directories(TEST PRIVATE include)
target_link_libraries(TEST gtest gtest_main)
gtest_discover_tests(TEST)
target_compile_definitions(TEST PUBLIC PROJECT_ROOT="${CMAKE_SOURCE_DIR}")
]]
add_executable(TCMP ${CPP_SOURCES_MAIN})
target_include_directories(TCMP PRIVATE include)
target_compile_definitions(TCMP PUBLIC PROJECT_ROOT="${CMAKE_SOURCE_DIR}")
add_compile_options(-Wall -Wextra -pedantic)
add_compile_options(-O0)