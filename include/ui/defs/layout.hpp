#pragma once
#include "traits.hpp"
#include <variant>

namespace core::ui {

    struct Flexbox {
        Direction direction = Direction::Column;
        Wrap wrap = Wrap::None;
        Justification justification = Justification::Start;
        Alignment items = Alignment::Stretch;
        Alignment self = Alignment::Auto;
        Alignment content = Alignment::Start;
        float grow = 0.0f;
        float shrink = 1.0f;
        float basis = -1.0f;
    };

    struct Grid {
        float columns = {0.0f};
        float rows = {0.0f};
        std::uint32_t spans[2] = {1, 1};
        std::uint32_t starts[2] = {0, 0};
        std::uint32_t counts[2] = {0, 0};
    };

    using Layout = std::variant<Flexbox, Grid>;

}