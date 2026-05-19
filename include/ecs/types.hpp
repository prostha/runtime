#pragma once
#include <cstddef>

namespace voxyl::ecs {

    // Forward declare Archetype so Record can store a pointer to it
    class Archetype;

    // Define the core Entity type and a NONE sentinel value
    using Entity = std::uint32_t;
    constexpr Entity NONE = 0xFFFFFFFF;

    // Defines where an entity's components are stored
    struct Record {
        Archetype* archetype = nullptr;
        std::size_t index = 0;
    };

}