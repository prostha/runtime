#include "../include/ui/node.hpp"
#include <yoga/Yoga.h>
#include <algorithm>

namespace core::ui {

    Node::Node() {
        yg_node = YGNodeNew();
    }

    Node::~Node() {
        if (yg_node) {
            YGNodeFree(yg_node);
        }
    }

    void Node::attach(Node* child) {
        if (!child || child->parent == this) return;

        if (child->parent) {
            child->parent->remove(child);
        }

        child->parent = this;
        children.push_back(child);

        const auto index = static_cast<uint32_t>(children.size() - 1);
        YGNodeInsertChild(yg_node, child->yg_node, index);
    }

    void Node::remove(Node* child) {
        if (!child || child->parent != this) return;

        if (const auto it = std::ranges::find(children, child); it != children.end()) {
            children.erase(it);
            YGNodeRemoveChild(yg_node, child->yg_node);
            child->parent = nullptr;
        }
    }

    void Node::compute(const float width, const float height) const {
        layout.compute(yg_node);

        for (const auto* child : children) {
            child->layout.compute(child->yg_node);
        }

        YGNodeCalculateLayout(yg_node, width, height, YGDirectionLTR);
    }

}