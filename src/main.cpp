#include <exception>
#include <fstream>
#include <iostream>
#include <string>

#include "codec.h"

namespace {

void print_usage(const char* prog_name) {
    std::cerr << "Usage:\n"
              << "  " << prog_name << " -c <input> <output>\n"
              << "  " << prog_name << " -d <input> <output>\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 4) {
        print_usage(argv[0]);
        return 1;
    }

    const std::string mode = argv[1];
    const std::string input_path = argv[2];
    const std::string output_path = argv[3];

    if (mode != "-c" && mode != "-d") {
        print_usage(argv[0]);
        return 1;
    }

    std::ifstream input(input_path, std::ios::binary);
    if (!input) {
        std::cerr << "Error: cannot open input file: " << input_path << "\n";
        return 1;
    }

    std::ofstream output(output_path, std::ios::binary);
    if (!output) {
        std::cerr << "Error: cannot open output file: " << output_path << "\n";
        return 1;
    }

    try {
        if (mode == "-c") {
            huffman::compress(input, output);
        } else {
            huffman::decompress(input, output);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
