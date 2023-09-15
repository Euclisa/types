cmake_minimum_required(VERSION 3.5)
project(types)
set (CMAKE_CXX_STANDARD 20)
find_package(GTest REQUIRED)
file(GLOB CPP_SOURCES_MAIN
                main.cpp
            )

file(GLOB CPP_SOURCES_TEST
                tests/uint.cpp
            )

add_executable(TEST ${CPP_SOURCES_TEST})
target_include_directories(TEST PRIVATE include)
target_link_libraries(TEST gtest gtest_main)
gtest_discover_tests(TEST)
target_compile_definitions(TEST PUBLIC PROJECT_ROOT="${CMAKE_SOURCE_DIR}")

add_executable(TYPES ${CPP_SOURCES_MAIN})
target_include_directories(TYPES PRIVATE include)
target_compile_definitions(TYPES PUBLIC PROJECT_ROOT="${CMAKE_SOURCE_DIR}")
add_compile_options(-Wall -Wextra -pedantic)
add_compile_options(-O0)