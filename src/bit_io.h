#pragma once

#include <cstdint>
#include <iosfwd>
#include <string_view>

namespace huffman {

class BitWriter {
  public:
    explicit BitWriter(std::ostream& out) : out_(out) {}

    void put_bit(bool bit);
    void put_bits(std::string_view bits);
    int flush();

  private:
    std::ostream& out_;
    std::uint8_t buffer_ = 0;
    int bits_in_buffer_ = 0;
};

class BitReader {
  public:
    explicit BitReader(std::istream& in) : in_(in) {}

    bool get_bit(bool& bit);

  private:
    std::istream& in_;
    std::uint8_t buffer_ = 0;
    int bits_left_ = 0;
};

}  // namespace huffman
