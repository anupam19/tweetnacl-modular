# CMakeLists.txt for GTest Tests
# Author: Anupam Datta <adbd04@gmail.com>

cmake_minimum_required(VERSION 3.10)
project(TweetNaCl_GTest VERSION 2.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/../../includes
    ${CMAKE_SOURCE_DIR}/../../arch
    ${CMAKE_SOURCE_DIR}/../../pqc
)

# TweetNaCl library sources
set(TWEETNACL_SOURCES
    ../../src/tweetnacl.c
    ../../src/randombytes.c
    ../../src/secure_mem.c
    ../../src/secure_utils.c
    ../../pqc/pqc.c
)

# Main test executable
add_executable(test_tweetnacl
    test_tweetnacl.cpp
    ${TWEETNACL_SOURCES}
)

target_link_libraries(test_tweetnacl
    GTest::gtest_main
    pthread
)

enable_testing()
add_test(NAME TweetNaClTests COMMAND test_tweetnacl)

# Coverage option
option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE)
    target_compile_options(test_tweetnacl PRIVATE --coverage -fprofile-arcs -ftest-coverage)
    target_link_options(test_tweetnacl PRIVATE --coverage)
endif()
