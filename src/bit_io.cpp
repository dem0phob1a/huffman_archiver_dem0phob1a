#include "bit_io.h"

#include <istream>
#include <ostream>

namespace huffman {

void BitWriter::put_bit(bool bit) {
    buffer_ = static_cast<std::uint8_t>((buffer_ << 1) | (bit ? 1 : 0));
    ++bits_in_buffer_;
    if (bits_in_buffer_ == 8) {
        out_.put(static_cast<char>(buffer_));
        buffer_ = 0;
        bits_in_buffer_ = 0;
    }
}

void BitWriter::put_bits(std::string_view bits) {
    for (char c : bits) {
        put_bit(c == '1');
    }
}

int BitWriter::flush() {
    if (bits_in_buffer_ == 0) {
        return 0;
    }
    int padding = 8 - bits_in_buffer_;
    buffer_ = static_cast<std::uint8_t>(buffer_ << padding);
    out_.put(static_cast<char>(buffer_));
    buffer_ = 0;
    bits_in_buffer_ = 0;
    return padding;
}

bool BitReader::get_bit(bool& bit) {
    if (bits_left_ == 0) {
        int byte = in_.get();
        if (byte == std::char_traits<char>::eof()) {
            return false;
        }
        buffer_ = static_cast<std::uint8_t>(byte);
        bits_left_ = 8;
    }
    --bits_left_;
    bit = ((buffer_ >> bits_left_) & 1) != 0;
    return true;
}

} // namespace huffman
