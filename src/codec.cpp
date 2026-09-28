#include "codec.h"

#include <istream>
#include <iterator>
#include <ostream>
#include <string>
#include <string_view>

#include "bit_io.h"
#include "huffman_tree.h"

namespace huffman {

namespace {

void write_u64(std::ostream& out, std::uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        out.put(static_cast<char>((value >> (8 * i)) & 0xFF));
    }
}

void write_u16(std::ostream& out, std::uint16_t value) {
    for (int i = 0; i < 2; ++i) {
        out.put(static_cast<char>((value >> (8 * i)) & 0xFF));
    }
}

std::uint64_t read_u64(std::istream& in) {
    std::uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        int byte = in.get();
        if (byte == std::char_traits<char>::eof()) {
            throw FormatError("unexpected end of file while reading a 64-bit header field");
        }
        value |= static_cast<std::uint64_t>(static_cast<unsigned char>(byte)) << (8 * i);
    }
    return value;
}

std::uint16_t read_u16(std::istream& in) {
    std::uint16_t value = 0;
    for (int i = 0; i < 2; ++i) {
        int byte = in.get();
        if (byte == std::char_traits<char>::eof()) {
            throw FormatError("unexpected end of file while reading a 16-bit header field");
        }
        value = static_cast<std::uint16_t>(value | (static_cast<unsigned char>(byte) << (8 * i)));
    }
    return value;
}

constexpr std::string_view kMagic = "HUF1";

} // namespace

void compress(std::istream& input, std::ostream& output) {
    std::string data((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());

    FrequencyTable frequencies{};
    for (unsigned char byte : data) {
        ++frequencies[byte];
    }

    HuffmanTree tree(frequencies);

    output.write(kMagic.data(), static_cast<std::streamsize>(kMagic.size()));
    write_u64(output, data.size());

    std::uint16_t distinct = 0;
    for (auto freq : frequencies) {
        if (freq > 0) {
            ++distinct;
        }
    }
    write_u16(output, distinct);
    for (int symbol = 0; symbol < 256; ++symbol) {
        auto freq = frequencies[static_cast<std::size_t>(symbol)];
        if (freq == 0) {
            continue;
        }
        output.put(static_cast<char>(symbol));
        write_u64(output, freq);
    }

    BitWriter writer(output);
    const CodeTable& codes = tree.codes();
    for (unsigned char byte : data) {
        writer.put_bits(codes.at(byte));
    }
    writer.flush();
}

void decompress(std::istream& input, std::ostream& output) {
    std::string magic(kMagic.size(), '\0');
    input.read(magic.data(), static_cast<std::streamsize>(magic.size()));
    if (!input || magic != kMagic) {
        throw FormatError("not a huffman archive: bad magic number");
    }

    std::uint64_t original_size = read_u64(input);
    std::uint16_t distinct = read_u16(input);

    FrequencyTable frequencies{};
    for (std::uint16_t i = 0; i < distinct; ++i) {
        int symbol = input.get();
        if (symbol == std::char_traits<char>::eof()) {
            throw FormatError("truncated frequency table");
        }
        std::uint64_t freq = read_u64(input);
        frequencies[static_cast<unsigned char>(symbol)] = freq;
    }

    if (original_size == 0) {
        return;
    }

    HuffmanTree tree(frequencies);
    const Node* root = tree.root();
    if (root == nullptr) {
        throw FormatError("archive header has zero distinct symbols but nonzero size");
    }

    BitReader reader(input);
    const Node* node = root;
    std::uint64_t produced = 0;
    bool bit = false;
    while (produced < original_size && reader.get_bit(bit)) {
        node = bit ? node->right.get() : node->left.get();
        if (node == nullptr) {
            throw FormatError("corrupted bitstream: walked off the Huffman tree");
        }
        if (node->is_leaf()) {
            output.put(static_cast<char>(node->symbol));
            ++produced;
            node = root;
        }
    }

    if (produced != original_size) {
        throw FormatError("truncated compressed data: fewer symbols decoded than expected");
    }
}

} // namespace huffman
