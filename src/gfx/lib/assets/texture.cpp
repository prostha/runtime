#include "gfx/lib/assets/texture.hpp"

namespace core::gfx::lib::assets {

    std::uint32_t Texture::load(const std::string_view path) noexcept {
        const std::string key(path);
        if (const auto match = registry.find(key); match != registry.end()) {
            return match->second;
        }
        const std::uint32_t id = static_cast<std::uint32_t>(storage.size());
        storage.push_back(Block{nullptr, 0, 0});
        registry[key] = id;
        return id;
    }

    std::uint32_t Texture::create(const std::uint8_t* pixels, const std::uint32_t width, const std::uint32_t height) noexcept {
        const std::uint32_t id = static_cast<std::uint32_t>(storage.size());
        storage.push_back(Block{const_cast<std::uint8_t*>(pixels), width, height});
        return id;
    }

}