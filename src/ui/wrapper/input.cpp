#include "../../../include/ui/wrapper/input.hpp"

static core::ui::Input input;

extern "C" {
    __declspec(dllexport) core::ui::Input* get() {
        return &input;
    }

    __declspec(dllexport) void clear() {
        input.scroll[0] = 0.0f;
        input.scroll[1] = 0.0f;
    }
}