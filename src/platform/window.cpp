#include "platform/window.hpp"

#if defined(_WIN32)
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C" __attribute__((visibility("default"))) __attribute__((used))
#endif

namespace platform {

    static Window display;

    const Window& window() {
        return display;
    }

}

EXPORT void resize(int width, int height) {
    if (width >= 0 && height >= 0) {
        platform::display.width = static_cast<uint32_t>(width);
        platform::display.height = static_cast<uint32_t>(height);
    }
}

EXPORT void focus(bool state) {
    platform::display.focus = state;
}