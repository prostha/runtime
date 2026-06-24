#pragma once
#include <cstdint>

namespace core::ui {

    enum class Ease : uint8_t {
        Linear,
        OutQuad,
        OutCubic,
        OutElastic
    };

    struct Track {
        float* target = nullptr;
        float start = 0.0f;
        float end = 0.0f;
        float duration = 0.0f;
        float elapsed = 0.0f;
        Ease ease = Ease::Linear;
        bool active = false;

        void update(float delta);
    };

    class Motion {
        static constexpr int MAX_TWEENS = 256;
        Track pool[MAX_TWEENS];

    public:
        Motion() = default;
        int instances = 0;

        void update(float delta);
        void create(float* target, float end, float duration, Ease ease);
    };

    extern Motion motion;
}
