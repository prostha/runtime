#pragma once
#include <cstdint>

namespace core::ui {

    enum class Positioning : std::uint8_t {
        Relative,
        Absolute
    };

    enum class Display : std::uint8_t {
        Flex,
        Grid,
        None
    };

    enum class Direction : std::uint8_t {
        Row,
        RowReverse,
        Column,
        ColumnReverse
    };

    enum class Wrap : std::uint8_t {
        None,
        Line,
        Reverse
    };

    enum class Justification : std::uint8_t {
        Start,
        Center,
        End,
        Between,
        Around,
        Evenly
    };

    enum class Alignment : std::uint8_t {
        Auto,
        Start,
        Center,
        End,
        Stretch,
        Baseline,
        Between,
        Around
    };

    enum class Policy : std::uint8_t {
        Relative,
        Absolute
    };

}