#pragma once
#include <cstdint>

typedef struct YGNode* YGNodeRef;

namespace core::ui {

    enum class Display : uint8_t { Flex, None };
    enum class Direction : uint8_t { Row, RowReverse, Column, ColumnReverse };
    enum class Position : uint8_t { Relative, Absolute };
    enum class Wrap : uint8_t { None, Line, Reverse };

    struct Size {
        float width = -1.0f;
        float height = -1.0f;
        float minimum[2] = {-1.0f, -1.0f};
        float maximum[2] = {-1.0f, -1.0f};
    };

    struct Flex {
        enum class Justify : uint8_t { Start, Center, End, Between, Around, Evenly };
        enum class Align : uint8_t { Auto, Start, Center, End, Stretch, Baseline, Between, Around };

        Direction direction = Direction::Column;
        Wrap wrap = Wrap::None;
        Justify justify = Justify::Start;
        Align items = Align::Stretch;
        Align self = Align::Auto;
        Align content = Align::Start;
        float grow = 0.0f;
        float shrink = 1.0f;
        float basis = -1.0f;
        float gap[2] = {0.0f, 0.0f};
    };

    struct Inbound {
        float padding[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    };

    struct Outbound {
        float margin[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    };

    struct Edge {
        Position mode = Position::Relative;
        float inset[4] = {-1.0f, -1.0f, -1.0f, -1.0f};
    };

    struct Layout {
        Display display = Display::Flex;
        Size size;
        Flex flex;
        Inbound inbound;
        Outbound outbound;
        Edge edge;

        void compute(YGNodeRef node) const;
    };

}