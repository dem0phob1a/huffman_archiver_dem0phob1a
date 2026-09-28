#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <utility>

#include "huffman_tree.h"

namespace {

huffman::FrequencyTable make_frequencies(
    std::initializer_list<std::pair<unsigned char, std::uint64_t>> entries) {
    huffman::FrequencyTable table{};
    for (const auto& [symbol, freq] : entries) {
        table[symbol] = freq;
    }
    return table;
}

}  // namespace

TEST(HuffmanTree, EmptyInputProducesNoTreeAndNoCodes) {
    huffman::FrequencyTable freq{};
    huffman::HuffmanTree tree(freq);
    EXPECT_EQ(tree.root(), nullptr);
    EXPECT_TRUE(tree.codes().empty());
}

TEST(HuffmanTree, SingleSymbolGetsOneBitCode) {
    auto freq = make_frequencies({{'a', 5}});
    huffman::HuffmanTree tree(freq);
    ASSERT_EQ(tree.codes().size(), 1u);
    EXPECT_EQ(tree.codes().at('a'), "0");
}

TEST(HuffmanTree, CodesArePrefixFree) {
    auto freq = make_frequencies({{'a', 45}, {'b', 13}, {'c', 12}, {'d', 16}, {'e', 9}, {'f', 5}});
    huffman::HuffmanTree tree(freq);
    const auto& codes = tree.codes();
    ASSERT_EQ(codes.size(), 6u);

    for (const auto& [symbol1, code1] : codes) {
        for (const auto& [symbol2, code2] : codes) {
            if (symbol1 == symbol2) {
                continue;
            }
            EXPECT_FALSE(code2.rfind(code1, 0) == 0)
                << "code for symbol " << static_cast<int>(symbol1)
                << " is a prefix of code for symbol " << static_cast<int>(symbol2);
        }
    }
}

TEST(HuffmanTree, MostFrequentSymbolGetsShortestOrEqualCode) {
    auto freq = make_frequencies({{'a', 45}, {'b', 13}, {'c', 12}, {'d', 16}, {'e', 9}, {'f', 5}});
    huffman::HuffmanTree tree(freq);
    const auto& codes = tree.codes();

    for (const auto& [symbol, code] : codes) {
        if (symbol == 'a') {
            continue;
        }
        EXPECT_LE(codes.at('a').size(), code.size());
    }
}

TEST(HuffmanTree, AverageCodeLengthMatchesTheoreticalOptimum) {
    const std::initializer_list<std::pair<unsigned char, std::uint64_t>> entries = {
        {'a', 45}, {'b', 13}, {'c', 12}, {'d', 16}, {'e', 9}, {'f', 5}};
    auto freq = make_frequencies(entries);
    huffman::HuffmanTree tree(freq);
    const auto& codes = tree.codes();

    std::uint64_t total_freq = 0;
    double total_bits = 0.0;
    for (const auto& [symbol, symbol_freq] : entries) {
        total_freq += symbol_freq;
        total_bits +=
            static_cast<double>(symbol_freq) * static_cast<double>(codes.at(symbol).size());
    }

    EXPECT_NEAR(total_bits / static_cast<double>(total_freq), 2.24, 1e-9);
}

TEST(HuffmanTree, HandlesFullByteAlphabet) {
    huffman::FrequencyTable freq{};
    for (int symbol = 0; symbol < 256; ++symbol) {
        freq[static_cast<std::size_t>(symbol)] = static_cast<std::uint64_t>(symbol + 1);
    }
    huffman::HuffmanTree tree(freq);
    EXPECT_EQ(tree.codes().size(), 256u);
    for (const auto& [symbol, code] : tree.codes()) {
        (void)symbol;
        EXPECT_FALSE(code.empty());
    }
}
