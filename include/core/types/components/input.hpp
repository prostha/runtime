#pragma once
#include <cstdint>

namespace core::ui {

    extern "C" {
        struct Input {
            // Bit flags for states:
            // Bit 0 (0x01) = Hovered (Mouse over or touch actively held on item)
            // Bit 1 (0x02) = Pressed (Mouse click, screen tap, or controller button down)
            // Bit 2 (0x04) = Focused (Currently receiving text/keyboard/gamepad inputs)
            // Bit 3 (0x08) = Disabled (Greyed out, ignores all interactions)
            uint32_t state = 0;

            // PC: Absolute mouse pointer coordinates on screen.
            // Mobile: Absolute screen position of the primary touch finger pointer contact point.
            // Console: Virtual cursor position or calculated interaction point from the analog stick.
            float pointer[2] = {0.0f, 0.0f};

            // Captures scrolling/dragging. Maps to mouse wheels, touch dragging, or gamepad scrolling
            float scroll[2] = {0.0f, 0.0f};

            // 0 = Released, 1 = Actively Pressed
            // Index mapping merges keyboard keys and console buttons:
            // Indices 0-255: Standard PC Virtual Keyboard Keys (A-Z, Enter, Space, Escape, etc.)
            // Indices 256-258: Mouse Buttons (0 = Left Click, 1 = Right Click, 2 = Middle Click)
            // Indices 259-275: Unified Console Gamepad (D-Pad, Face Buttons A/B/X/Y, Triggers, Bumpers)
            uint8_t keys[280] = {};
        };
    }

}