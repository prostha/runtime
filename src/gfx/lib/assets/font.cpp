#include "gfx/lib/assets/font.hpp"

namespace core::gfx::lib::assets {

    Font::Font(Texture& texture_registry) noexcept : textures(texture_registry) {}

    std::uint32_t Font::load(const std::string_view path) noexcept {
        const std::uint32_t id = this->textures.load(path);
        this->storage.push_back({id, {}});
        return static_cast<std::uint32_t>(this->storage.size());
    }

    const Glyph* Font::find(const std::uint32_t id, const char32_t code) const noexcept {
        if (id == 0 || id > this->storage.size()) {
            return nullptr;
        }
        for (const auto& glyph : this->storage[id - 1].glyphs) {
            if (glyph.code == code) {
                return &glyph;
            }
        }
        return nullptr;
    }

    std::uint32_t Font::atlas(const std::uint32_t id) const noexcept {
        return (id > 0 && id <= this->storage.size()) ? this->storage[id - 1].atlas_identifier : 0;
    }

    void Font::dispose(const std::uint32_t id) noexcept {
        if (id > 0 && id <= this->storage.size()) {
            this->storage[id - 1] = {};
        }
    }

    void Font::clear() noexcept {
        this->storage.clear();
    }

}