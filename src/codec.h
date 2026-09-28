#pragma once

#include <iosfwd>
#include <stdexcept>

namespace huffman {

class FormatError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

void compress(std::istream& input, std::ostream& output);
void decompress(std::istream& input, std::ostream& output);

} // namespace huffman
