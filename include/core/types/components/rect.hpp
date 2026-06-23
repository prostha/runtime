#pragma once

#include "core/types/primitives/vector2.hpp"

namespace core::components {

    struct Rect {
        primitives::Vector2 position{0.0f, 0.0f};
        float rotation{0.0f};
        primitives::Vector2 scale{1.0f, 1.0f};
        primitives::Vector2 anchor{0.0f, 0.0f};
    };

}