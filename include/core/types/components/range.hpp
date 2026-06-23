#pragma once

namespace core::components {
    struct Range {
        float min{0.0f};
        float max{1.0f};
        float current{0.0f};
        float step{0.0f};
    };
}