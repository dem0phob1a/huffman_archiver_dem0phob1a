#include "huffman_tree.h"

#include <queue>
#include <vector>

namespace huffman {

namespace {

struct QueueEntry {
    Node* node;
    std::uint64_t order;
};

struct QueueEntryCompare {
    bool operator()(const QueueEntry& left, const QueueEntry& right) const noexcept {
        if (left.node->frequency != right.node->frequency) {
            return left.node->frequency > right.node->frequency;
        }
        return left.order > right.order;
    }
};

}  // namespace

HuffmanTree::HuffmanTree(const FrequencyTable& frequencies) {
    build(frequencies);
    if (root_) {
        std::string path;
        assign_codes(root_, path);
    }
}

void HuffmanTree::build(const FrequencyTable& frequencies) {
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, QueueEntryCompare> forest;
    std::uint64_t order = 0;
    const auto create_node = [this]() {
        nodes_.push_back(std::make_unique<Node>());
        return nodes_.back().get();
    };

    for (int symbol = 0; symbol < 256; ++symbol) {
        if (frequencies[static_cast<std::size_t>(symbol)] == 0) {
            continue;
        }
        Node* leaf = create_node();
        leaf->frequency = frequencies[static_cast<std::size_t>(symbol)];
        leaf->symbol = symbol;
        forest.push({leaf, order++});
    }

    if (forest.empty()) {
        return;
    }

    if (forest.size() == 1) {
        Node* parent = create_node();
        parent->frequency = forest.top().node->frequency;
        parent->left = forest.top().node;
        root_ = parent;
        return;
    }

    while (forest.size() > 1) {
        Node* left = forest.top().node;
        forest.pop();
        Node* right = forest.top().node;
        forest.pop();

        Node* parent = create_node();
        parent->frequency = left->frequency + right->frequency;
        parent->left = left;
        parent->right = right;
        forest.push({parent, order++});
    }

    root_ = forest.top().node;
}

void HuffmanTree::assign_codes(const Node* node, std::string& path) {
    if (node->is_leaf()) {
        codes_[static_cast<std::uint8_t>(node->symbol)] = path;
        return;
    }
    if (node->left) {
        path.push_back('0');
        assign_codes(node->left, path);
        path.pop_back();
    }
    if (node->right) {
        path.push_back('1');
        assign_codes(node->right, path);
        path.pop_back();
    }
}

}  // namespace huffman
