#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

namespace huffman {

struct Node {
    std::uint64_t frequency = 0;
    int symbol = -1;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;

    bool is_leaf() const noexcept { return symbol >= 0; }
};

using FrequencyTable = std::array<std::uint64_t, 256>;
using CodeTable = std::unordered_map<std::uint8_t, std::string>;

class HuffmanTree {
public:
    explicit HuffmanTree(const FrequencyTable& frequencies);

    const CodeTable& codes() const noexcept { return codes_; }
    const Node* root() const noexcept { return root_.get(); }

private:
    void build(const FrequencyTable& frequencies);
    void assign_codes(const Node* node, std::string& path);

    std::unique_ptr<Node> root_;
    CodeTable codes_;
};

} // namespace huffman
