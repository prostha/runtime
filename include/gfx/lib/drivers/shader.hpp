#pragma once
#include <cstddef>

namespace core::gfx::lib::drivers {

    struct Shader {
        const void* code{nullptr};
        std::size_t size{0};
    };

}