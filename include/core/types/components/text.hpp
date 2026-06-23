#pragma once
#include <string>

namespace core::components {
    struct Text {
        std::string content;
        unsigned int font{0};
        float size{14.0f};
        float color[3]{0.0f, 0.0f, 0.0f};
        float opacity{1.0f};
        int alignment{0};
    };
}