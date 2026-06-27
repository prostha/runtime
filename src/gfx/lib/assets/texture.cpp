#include "gfx/lib/assets/texture.hpp"

namespace core::gfx::lib::assets {

    std::uint32_t Texture::load(const std::string_view path) noexcept {
        const std::string key(path);
        if (const auto item = registry.find(key); item != registry.end()) {
            return item->second;
        }
        storage.push_back({nullptr, 512, 512});
        const auto id = static_cast<std::uint32_t>(storage.size());
        registry[key] = id;
        return id;
    }

    std::uint32_t Texture::create(const std::uint8_t* pixels, const std::uint32_t width, const std::uint32_t height) noexcept {
        storage.push_back({pixels, width, height});
        return static_cast<std::uint32_t>(storage.size());
    }

    void Texture::update(const std::uint32_t id, const std::uint8_t* pixels, const std::uint32_t width, const std::uint32_t height) noexcept {
        if (id > 0 && id <= storage.size()) {
            storage[id - 1] = {pixels, width, height};
        }
    }

    const Texture::Block* Texture::get(const std::uint32_t id) const noexcept {
        return (id > 0 && id <= storage.size()) ? &storage[id - 1] : nullptr;
    }

    void Texture::dispose(const std::uint32_t id) noexcept {
        if (id > 0 && id <= storage.size()) {
            storage[id - 1] = {};
        }
    }

    void Texture::clear() noexcept {
        storage.clear();
        registry.clear();
    }

}