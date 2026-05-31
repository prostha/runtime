#pragma once

#include <cstdint>

namespace platform {

    struct Window {
        uint32_t width = 1280;
        uint32_t height = 720;
        bool focus = true;
    };

    const Window& window();
}