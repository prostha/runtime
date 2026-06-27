#include "gfx/lib/assets/texture.hpp"

namespace core::gfx::lib::assets {

    std::uint32_t Texture::load(std::string_view path) noexcept {
        const std::string lookup_key(path);
        if (const auto iterator = registry.find(lookup_key); iterator != registry.end()) {
            return iterator->second;
        }
        storage.push_back({nullptr, 512, 512});
        const auto identifier = static_cast<std::uint32_t>(storage.size());
        registry[lookup_key] = identifier;
        return identifier;
    }

    std::uint32_t Texture::create(const std::uint8_t* pixels, std::uint32_t width, std::uint32_t height) noexcept {
        storage.push_back({const_cast<std::uint8_t*>(pixels), width, height});
        return static_cast<std::uint32_t>(storage.size());
    }

    void Texture::update(std::uint32_t id, const std::uint8_t* pixels, std::uint32_t width, std::uint32_t height) noexcept {
        if (id > 0 && id <= storage.size()) {
            storage[id - 1] = {const_cast<std::uint8_t*>(pixels), width, height};
        }
    }

    const Texture::Block* Texture::get(std::uint32_t id) const noexcept {
        return (id > 0 && id <= storage.size()) ? &storage[id - 1] : nullptr;
    }

    void Texture::dispose(std::uint32_t id) noexcept {
        if (id > 0 && id <= storage.size()) {
            storage[id - 1] = {};
        }
    }

    void Texture::clear() noexcept {
        registry.clear();
        storage.clear();
    }

}