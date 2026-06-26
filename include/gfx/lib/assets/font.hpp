#pragma once
#include "glyph.hpp"
#include "texture.hpp"
#include <string_view>
#include <vector>
#include <cstdint>

namespace core::gfx::lib::assets {

    class Font {
    public:
        struct Block {
            std::uint32_t atlas_id{0};
            std::vector<Glyph> glyphs{};
        };

        explicit Font(Texture& instance) noexcept : textures(instance) {}
        ~Font() = default;

        Font(const Font&) = delete;
        Font& operator=(const Font&) = delete;
        Font(Font&&) noexcept = default;
        Font& operator=(Font&&) noexcept = delete;

        std::uint32_t load(std::string_view path, float size) noexcept;

        [[nodiscard]] const Glyph* find(const std::uint32_t id, const char32_t code) const noexcept {
            if (id >= storage.size()) return nullptr;
            for (const auto& glyph : storage[id].glyphs) {
                if (glyph.code == code) return &glyph;
            }
            return nullptr;
        }

        [[nodiscard]] std::uint32_t atlas(const std::uint32_t id) const noexcept {
            return id < storage.size() ? storage[id].atlas_id : 0;
        }

        void dispose(const std::uint32_t id) noexcept {
            if (id < storage.size()) storage[id] = {};
        }

        void clear() noexcept {
            storage.clear();
        }

    private:
        Texture& textures;
        std::vector<Block> storage;
    };

}