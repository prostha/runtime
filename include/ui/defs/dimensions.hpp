#pragma once
#include "traits.hpp"

namespace core::ui {

    struct Dimensions {
        Policy policy = Policy::Relative;
        float width = -1.0f;
        float height = -1.0f;
        float minimum = {-1.0f, -1.0f};
        float maximum = {-1.0f, -1.0f};
        float inset = {-1.0f, -1.0f, -1.0f, -1.0f};
    };

}