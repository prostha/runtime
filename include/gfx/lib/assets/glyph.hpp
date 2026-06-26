#pragma once

namespace core::gfx::lib::assets {

    struct Glyph {
        char32_t code;
        float uv[4];
        float size[2];
        float bearing[2];
        float advance;
    };

}