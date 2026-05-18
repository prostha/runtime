#pragma once
#include <memory>

using Entity = uint32_t;
inline constexpr Entity NONE = 0xFFFFFFFF;

// Represents where an entity lives in memory right now
struct Record {
    class Archetype* archetype = nullptr;
    size_t index = 0;
};