#pragma once

#include "core/types/primitives/vector2.hpp"

namespace core::components {

    struct Bounds {
        primitives::Vector2 size{0.0f, 0.0f};
        primitives::Vector2 margin{0.0f, 0.0f};
        primitives::Vector2 padding{0.0f, 0.0f};
    };

}