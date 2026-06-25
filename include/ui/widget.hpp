#pragma once
#include "ui/defs/traits.hpp"
#include "ui/defs/dimensions.hpp"
#include "ui/defs/layout.hpp"
#include "ui/defs/spacing.hpp"
#include "defs/style.hpp"
#include <vector>

typedef struct YGNode* YGNodeRef;

namespace core::ui {

    using Id = std::uint32_t;

    struct Widget {
        YGNodeRef yg_node = nullptr;

        Display display = Display::Flex;
        Dimensions dimensions;
        Layout layout = Flexbox{};
        Style style;

        Surroundings padding = {0.0f, 0.0f, 0.0f, 0.0f};
        Surroundings margin = {0.0f, 0.0f, 0.0f, 0.0f};
        Axis gap = {0.0f, 0.0f};

        Id parent = 0;
        std::vector<Id> children;

        Widget();
        ~Widget();

        Widget(const Widget&) = delete;
        Widget& operator=(const Widget&) = delete;
        Widget(Widget&&) noexcept = default;
        Widget& operator=(Widget&&) noexcept = default;

        void compute(float width, float height, const auto& lookup) const;
    };

}