#pragma once

#include <cstddef>

namespace platform {

    constexpr size_t LIMIT = 512;

    struct Input {
        double x = 0.0;
        double y = 0.0;
        bool keys[LIMIT] = { false };
    };

    const Input& input();
    bool pressed(int code);

}