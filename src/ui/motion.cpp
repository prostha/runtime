#include "ui/motion.hpp"
#include <cmath>
#include <numbers>

core::ui::Motion motion;

namespace core::ui {

    struct Easing {
        static float calculate(const Ease type, const float time) {
            switch (type) {
                case Ease::Linear: return time;
                case Ease::OutQuad: return time * (2.0f - time);
                case Ease::OutCubic: {
                    const float offset = time - 1.0f;
                    return offset * offset * offset + 1.0f;
                }
                case Ease::OutElastic: {
                    if (time == 0.0f || time == 1.0f) return time;
                    constexpr float pi = std::numbers::pi_v<float>;
                    return std::pow(2.0f, -10.0f * time) * std::sin((time - 0.075f) * (2.0f * pi) / 0.3f) + 1.0f;
                }
                default: return time;
            }
        }
    };

    void Track::update(const float delta) {
        if (!active || !target) return;

        elapsed += delta;

        if (elapsed >= duration) {
            *target = end;
            active = false;
            return;
        }

        const float progress = elapsed / duration;
        const float blend = Easing::calculate(ease, progress);
        *target = start + (end - start) * blend;
    }

    void Motion::update(const float delta) {
        for (auto& track : pool) {
            if (track.active) {
                track.update(delta);
                if (!track.active) {
                    instances--;
                }
            }
        }
    }

    void Motion::create(float* target, const float end, const float duration, const Ease ease) {
        if (!target) return;

        for (auto& track : pool) {
            if (track.active && track.target == target) {
                track.start = *target;
                track.end = end;
                track.duration = duration;
                track.elapsed = 0.0f;
                track.ease = ease;
                return;
            }
        }

        for (auto& track : pool) {
            if (!track.active) {
                track.target = target;
                track.start = *target;
                track.end = end;
                track.duration = duration;
                track.elapsed = 0.0f;
                track.ease = ease;
                track.active = true;
                instances++;
                return;
            }
        }
    }

}