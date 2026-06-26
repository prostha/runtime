#pragma once
#include <cstdint>
#include <cstddef>

namespace core::gfx::lib::drivers {

    enum class Pipeline : std::uint16_t {
        Default = 0,
        Vector  = 1,
        Opaque  = 2,
        Shadow  = 3,
        Raycast = 4
    };

    enum class Type : std::uint8_t {
        Rect,
        Circle,
        Mesh,
        Text,
        Scissor
    };

    enum Flags : std::uint32_t {
        None  = 0,
        Alpha = 1 << 0,
        Test  = 1 << 1,
        Write = 1 << 2,
        Cull  = 1 << 3,
        Wire  = 1 << 4
    };

    union Key {
        std::uint64_t value;
        struct {
            std::uint64_t entity   : 16;
            std::uint64_t depth    : 24;
            std::uint64_t texture  : 12;
            std::uint64_t pipeline : 8;
            std::uint64_t pass     : 4;
        } bits;
    };

    struct Command {
        Type type;
        std::uint8_t gap[3];
        float box[4];
        std::uint32_t primary;
        std::uint32_t secondary;
        float data[4];
        const void* pointer;
        std::uint32_t size;
    };

    struct Buffer {
        const Key* keys;
        const Command* commands;
        std::size_t count;
    };

}