#pragma once

#include <cstdint>
#include <cstddef>

namespace core::gfx::lib::drivers {

    struct Rect {
        float x{0.0f};
        float y{0.0f};
        float width{0.0f};
        float height{0.0f};
    };

    enum class Pipeline : std::uint16_t {
        Default = 0,
        Vector = 1,
        Opaque = 2,
        Shadow = 3,
        Raycast = 4,
        Deferred = 5,
        Compute = 6,
        Post = 7
    };

    enum class Type : std::uint8_t {
        Rect,
        Circle,
        Mesh,
        Text,
        Scissor,
        Compute,
        Line,
        Volume,
        Instance
    };

    enum Flags : std::uint32_t {
        None = 0,
        Alpha = 1 << 0,
        Test = 1 << 1,
        Write = 1 << 2,
        Cull = 1 << 3,
        Wire = 1 << 4,
        Depth = 1 << 5,
        Stencil = 1 << 6,
        Blend = 1 << 7
    };

    union Key {
        std::uint64_t value;
        struct {
            std::uint64_t entity : 16;
            std::uint64_t depth : 24;
            std::uint64_t texture : 12;
            std::uint64_t pipeline : 8;
            std::uint64_t pass : 4;
        } bits;
    };

    struct Command {
        Type type;
        std::uint8_t gap[3];
        float box[4];
    };

    struct Buffer {
        const Key* keys;
        const Command* commands;
        std::size_t count;
    };

}