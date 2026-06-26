#pragma once
#include "ui/defs/traits.hpp"
#include <vector>
#include <cstdint>

typedef struct YGNode* YGNodeRef;

namespace core {
    class World;
}

namespace core::ui {

    using Id = std::uint32_t;

    struct Widget {
        YGNodeRef node = nullptr;
        Display display = Display::Flex;
        Positioning policy = Positioning::Relative;
        Id parent = 0;
        std::vector<Id> children;

        Widget();
        ~Widget();

        Widget(const Widget&) = delete;
        Widget& operator=(const Widget&) = delete;
        Widget(Widget&&) noexcept = default;
        Widget& operator=(Widget&&) noexcept = default;

        void compute(float width, float height, const World& registry) const;
    };

}