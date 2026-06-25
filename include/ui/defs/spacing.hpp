#pragma once
#include <variant>

namespace core::ui {

    struct Surroundings {
        float top    = 0.0f;
        float right  = 0.0f;
        float bottom = 0.0f;
        float left   = 0.0f;
    };

    struct Axis {
        float main  = 0.0f;
        float cross = 0.0f;
    };

    using Spacing = std::variant<Surroundings, Axis>;

}