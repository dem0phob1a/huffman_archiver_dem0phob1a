cmake_minimum_required(VERSION 3.16)
project(huffman_archiver CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(HUFFMAN_BUILD_TESTS "Build unit and integration tests" ON)
option(HUFFMAN_ENABLE_SANITIZERS "Build with AddressSanitizer + UndefinedBehaviorSanitizer" OFF)

if(HUFFMAN_ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

add_subdirectory(src)

if(HUFFMAN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
