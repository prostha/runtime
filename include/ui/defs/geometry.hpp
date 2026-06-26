#pragma once

namespace core::ui {

    struct Geometry {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    struct Constraints {
        float width = -1.0f;
        float height = -1.0f;
        float minimum[2] = {-1.0f, -1.0f};
        float maximum[2] = {-1.0f, -1.0f};
    };

}