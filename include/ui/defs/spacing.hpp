#pragma once

namespace core::ui {

    struct Margin {
        float top = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;
        float left = 0.0f;
    };

    struct Padding {
        float top = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;
        float left = 0.0f;
    };

    struct Gap {
        float main = 0.0f;
        float cross = 0.0f;
    };
}