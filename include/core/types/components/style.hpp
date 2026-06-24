#pragma once

#include <variant>
#include <cstdint>

namespace core::ui {

    struct Oklch {
        float l;
        float c;
        float h;
    };

    struct Rgb {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    using Color = std::variant<Rgb, Oklch>;

    struct Style {
        Color background = Rgb{0, 0, 0};
        float opacity = 1.0f;
    };

}