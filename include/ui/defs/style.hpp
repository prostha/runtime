#pragma once
#include <variant>
#include <cstdint>

namespace core::ui {

    struct Oklch {
        float lightness;
        float chroma;
        float hue;
    };

    struct Rgb {
        std::uint8_t red;
        std::uint8_t green;
        std::uint8_t blue;
    };

    using Color = std::variant<Rgb, Oklch>;

    struct Style {
        Color background = Rgb{0, 0, 0};
    };

}