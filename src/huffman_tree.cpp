#include "huffman_tree.h"

#include <algorithm>
#include <vector>

namespace huffman {

HuffmanTree::HuffmanTree(const FrequencyTable& frequencies) {
    build(frequencies);
    if (root_) {
        std::string path;
        assign_codes(root_.get(), path);
    }
}

void HuffmanTree::build(const FrequencyTable& frequencies) {
    std::vector<std::unique_ptr<Node>> forest;
    forest.reserve(256);
    for (int symbol = 0; symbol < 256; ++symbol) {
        if (frequencies[static_cast<std::size_t>(symbol)] == 0) {
            continue;
        }
        auto leaf = std::make_unique<Node>();
        leaf->frequency = frequencies[static_cast<std::size_t>(symbol)];
        leaf->symbol = symbol;
        forest.push_back(std::move(leaf));
    }

    if (forest.empty()) {
        return;
    }

    if (forest.size() == 1) {
        auto parent = std::make_unique<Node>();
        parent->frequency = forest[0]->frequency;
        parent->left = std::move(forest[0]);
        root_ = std::move(parent);
        return;
    }

    while (forest.size() > 1) {
        std::size_t first = 0;
        std::size_t second = 1;
        if (forest[second]->frequency < forest[first]->frequency) {
            std::swap(first, second);
        }
        for (std::size_t i = 2; i < forest.size(); ++i) {
            if (forest[i]->frequency < forest[first]->frequency) {
                second = first;
                first = i;
            } else if (forest[i]->frequency < forest[second]->frequency) {
                second = i;
            }
        }

        auto parent = std::make_unique<Node>();
        parent->frequency = forest[first]->frequency + forest[second]->frequency;
        parent->left = std::move(forest[first]);
        parent->right = std::move(forest[second]);

        std::size_t hi = std::max(first, second);
        std::size_t lo = std::min(first, second);
        forest.erase(forest.begin() + static_cast<std::ptrdiff_t>(hi));
        forest.erase(forest.begin() + static_cast<std::ptrdiff_t>(lo));
        forest.push_back(std::move(parent));
    }

    root_ = std::move(forest.front());
}

void HuffmanTree::assign_codes(const Node* node, std::string& path) {
    if (node->is_leaf()) {
        codes_[static_cast<std::uint8_t>(node->symbol)] = path;
        return;
    }
    if (node->left) {
        path.push_back('0');
        assign_codes(node->left.get(), path);
        path.pop_back();
    }
    if (node->right) {
        path.push_back('1');
        assign_codes(node->right.get(), path);
        path.pop_back();
    }
}

} // namespace huffman
