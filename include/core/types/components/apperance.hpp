#pragma once

namespace core::components {

    struct Appearance {

        struct Background {
            float color[3]{1.0f, 1.0f, 1.0f};
            float opacity{1.0f};
            unsigned int texture{0};
            float offset[2]{0.0f, 0.0f};
            float scale[2]{1.0f, 1.0f};
        } background;

        struct Border {
            float color[3]{0.0f, 0.0f, 0.0f};
            float opacity{1.0f};
            float thickness{0.0f};
            float radius{0.0f};
        } border;

        struct Shadow {
            float color[3]{0.0f, 0.0f, 0.0f};
            float opacity{0.0f};
            float offset[2]{0.0f, 0.0f};
            float blur{0.0f};
        } shadow;

        struct Effect {
            float blur{0.0f};
            float brightness{1.0f};
            bool clip{false};
        } effect;
    };

}