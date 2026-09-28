add_library(huffman_core
    huffman_tree.cpp
    bit_io.cpp
    codec.cpp
)
target_include_directories(huffman_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(huffman_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(huffman_archiver main.cpp)
target_link_libraries(huffman_archiver PRIVATE huffman_core)
target_compile_options(huffman_archiver PRIVATE -Wall -Wextra -Wpedantic)
