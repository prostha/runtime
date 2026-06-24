#pragma once
#include "layout.hpp"
#include <vector>

namespace core::ui {

    struct Node {
        YGNodeRef yg_node = nullptr;
        Layout layout;
        Node* parent = nullptr;
        std::vector<Node*> children;

        Node();
        ~Node();

        void attach(Node* child);
        void remove(Node* child);
        void compute(float width, float height) const;
    };

}