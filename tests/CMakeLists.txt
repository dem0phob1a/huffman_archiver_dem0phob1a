include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(huffman_tests
    test_huffman_tree.cpp
    test_bit_io.cpp
    test_codec.cpp
)
target_link_libraries(huffman_tests PRIVATE huffman_core GTest::gtest_main)
target_compile_options(huffman_tests PRIVATE -Wall -Wextra -Wpedantic)

include(GoogleTest)
gtest_discover_tests(huffman_tests)
