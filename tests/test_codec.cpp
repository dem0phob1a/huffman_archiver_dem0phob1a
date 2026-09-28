#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "codec.h"

namespace {

std::string round_trip(const std::string& data) {
    std::istringstream original(data);
    std::ostringstream compressed;
    huffman::compress(original, compressed);

    std::istringstream compressed_in(compressed.str());
    std::ostringstream decompressed;
    huffman::decompress(compressed_in, decompressed);
    return decompressed.str();
}

}  // namespace

TEST(Codec, RoundTripEmptyInput) { EXPECT_EQ(round_trip(""), ""); }

TEST(Codec, RoundTripSingleRepeatedByte) {
    std::string data(1000, 'x');
    EXPECT_EQ(round_trip(data), data);
}

TEST(Codec, RoundTripPlainText) {
    std::string text =
        "the quick brown fox jumps over the lazy dog. "
        "the quick brown fox jumps over the lazy dog.";
    EXPECT_EQ(round_trip(text), text);
}

TEST(Codec, RoundTripAllByteValuesIncludingZero) {
    std::string data;
    data.reserve(256 * 4);
    for (int value = 0; value < 256; ++value) {
        for (int repeat = 0; repeat < 4; ++repeat) {
            data.push_back(static_cast<char>(value));
        }
    }
    EXPECT_EQ(round_trip(data), data);
}

TEST(Codec, CompressedSizeIsSmallerForSkewedFrequencies) {
    std::string text(10000, 'a');
    for (std::size_t i = 0; i < text.size(); i += 137) {
        text[i] = 'b';
    }

    std::istringstream original(text);
    std::ostringstream compressed;
    huffman::compress(original, compressed);

    EXPECT_LT(compressed.str().size(), text.size());
    EXPECT_EQ(round_trip(text), text);
}

TEST(Codec, DecompressRejectsBadMagicNumber) {
    std::istringstream bad("NOTHUFF is not a valid archive");
    std::ostringstream out;
    EXPECT_THROW(huffman::decompress(bad, out), huffman::FormatError);
}

TEST(Codec, DecompressRejectsTruncatedHeader) {
    std::istringstream bad("HUF1\x01\x02");
    std::ostringstream out;
    EXPECT_THROW(huffman::decompress(bad, out), huffman::FormatError);
}
