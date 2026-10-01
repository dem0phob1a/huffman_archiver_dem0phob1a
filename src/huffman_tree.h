#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace huffman {

struct Node {
    std::uint64_t frequency = 0;
    int symbol = -1;
    Node* left = nullptr;
    Node* right = nullptr;

    bool is_leaf() const noexcept { return symbol >= 0; }
};

using FrequencyTable = std::array<std::uint64_t, 256>;
using CodeTable = std::unordered_map<std::uint8_t, std::string>;

class HuffmanTree {
  public:
    explicit HuffmanTree(const FrequencyTable& frequencies);

    const CodeTable& codes() const noexcept { return codes_; }
    const Node* root() const noexcept { return root_; }

  private:
    void build(const FrequencyTable& frequencies);
    void assign_codes(const Node* node, std::string& path);

    std::vector<std::unique_ptr<Node>> nodes_;
    const Node* root_ = nullptr;
    CodeTable codes_;
};

}  // namespace huffman
