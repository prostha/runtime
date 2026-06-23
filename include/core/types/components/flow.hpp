#pragma once

namespace core::components {
    struct Flow {
        int axis{0};
        int distribution{0};
        int alignment{0};
        float gap{0.0f};
        bool wrap{false};
    };
}