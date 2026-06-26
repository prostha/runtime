#pragma once
#include <cstdint>

namespace core {
    class World;
}

namespace core::ui {

    void dispatch(const void* handle, const World* world, std::uint32_t tag);

}