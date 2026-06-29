#include "gfx/lib/assets/font.hpp"

namespace core::gfx::lib::assets {

    Font::Font(Texture& registry) noexcept : textures(registry) {}

    std::uint32_t Font::load(const std::string_view path) noexcept {
        const auto id = textures.load(path);
        storage.push_back({id, {}});
        return static_cast<std::uint32_t>(storage.size());
    }

    const Font::Block* Font::get(const std::uint32_t id) const noexcept {
        return id > 0 && id <= storage.size() ? &storage[id - 1] : nullptr;
    }

    void Font::dispose(const std::uint32_t id) noexcept {
        if (id > 0 && id <= storage.size()) {
            if (storage[id - 1].atlas != 0) {
                textures.dispose(storage[id - 1].atlas);
            }
            storage[id - 1] = {};
        }
    }

    void Font::clear() noexcept {
        for (auto& [atlas, glyphs] : storage) {
            if (atlas != 0) {
                textures.dispose(atlas);
            }
        }
        storage.clear();
    }

}