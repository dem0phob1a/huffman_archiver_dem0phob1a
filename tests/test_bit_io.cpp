#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "bit_io.h"

TEST(BitWriter, WritesBitsMostSignificantFirstAndPadsOnFlush) {
    std::ostringstream out;
    {
        huffman::BitWriter writer(out);
        writer.put_bits("10101");
        int padding = writer.flush();
        EXPECT_EQ(padding, 3);
    }
    ASSERT_EQ(out.str().size(), 1u);
    EXPECT_EQ(static_cast<unsigned char>(out.str()[0]), 0b10101000);
}

TEST(BitWriter, FlushOnByteBoundaryAddsNoPadding) {
    std::ostringstream out;
    huffman::BitWriter writer(out);
    writer.put_bits("11110000");
    EXPECT_EQ(writer.flush(), 0);
    ASSERT_EQ(out.str().size(), 1u);
    EXPECT_EQ(static_cast<unsigned char>(out.str()[0]), 0b11110000);
}

TEST(BitWriter, FlushWithEmptyBufferWritesNothing) {
    std::ostringstream out;
    huffman::BitWriter writer(out);
    EXPECT_EQ(writer.flush(), 0);
    EXPECT_TRUE(out.str().empty());
}

TEST(BitReader, ReadsBitsWrittenByBitWriter) {
    std::ostringstream out;
    {
        huffman::BitWriter writer(out);
        writer.put_bits("1100101101");
        writer.flush();
    }

    std::istringstream in(out.str());
    huffman::BitReader reader(in);
    std::string expected = "1100101101";
    std::string actual;
    bool bit = false;
    for (std::size_t i = 0; i < expected.size(); ++i) {
        ASSERT_TRUE(reader.get_bit(bit));
        actual.push_back(bit ? '1' : '0');
    }
    EXPECT_EQ(actual, expected);
}

TEST(BitReader, ReturnsFalseAtEndOfEmptyStream) {
    std::istringstream in("");
    huffman::BitReader reader(in);
    bool bit = false;
    EXPECT_FALSE(reader.get_bit(bit));
}

TEST(BitReader, ReturnsFalseExactlyAfterLastByteIsConsumed) {
    std::ostringstream out;
    {
        huffman::BitWriter writer(out);
        writer.put_bits("11111111");
        writer.flush();
    }
    std::istringstream in(out.str());
    huffman::BitReader reader(in);
    bool bit = false;
    for (int i = 0; i < 8; ++i) {
        ASSERT_TRUE(reader.get_bit(bit));
    }
    EXPECT_FALSE(reader.get_bit(bit));
}
