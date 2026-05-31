#include "platform/input.hpp"

#if defined(_WIN32)
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C" __attribute__((visibility("default"))) __attribute__((used))
#endif

namespace platform {

    static Input device;

    const Input& input() {
        return device;
    }

    bool pressed(int code) {
        if (code >= 0 && static_cast<size_t>(code) < LIMIT) {
            return device.keys[code];
        }
        return false;
    }

}

EXPORT void key(int code, bool pressed) {
    if (code >= 0 && static_cast<size_t>(code) < platform::LIMIT) {
        platform::device.keys[code] = pressed;
    }
}

EXPORT void move(double x, double y) {
    platform::device.x = x;
    platform::device.y = y;
}