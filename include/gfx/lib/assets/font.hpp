#pragma once
#include <string_view>
#include <vector>
#include <cstdint>

#include "gfx/lib/assets/glyph.hpp"
#include "gfx/lib/assets/texture.hpp"

namespace core::gfx::lib::assets {

    class Font final {
    public:
        struct Block {
            std::uint32_t atlas_identifier{0};
            std::vector<Glyph> glyphs;
        };

        explicit Font(Texture& texture_registry) noexcept;
        ~Font() noexcept = default;

        Font(const Font&) = delete;
        Font& operator=(const Font&) = delete;
        Font(Font&&) noexcept = default;
        Font& operator=(Font&&) noexcept = delete;

        std::uint32_t load(std::string_view path) noexcept;
        [[nodiscard]] const Glyph* find(std::uint32_t id, char32_t code) const noexcept;
        [[nodiscard]] std::uint32_t atlas(std::uint32_t id) const noexcept;
        void dispose(std::uint32_t id) noexcept;
        void clear() noexcept;

    private:
        Texture& textures;
        std::vector<Block> storage;
    };

}