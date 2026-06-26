#include "gfx/lib/assets/font.hpp"

namespace core::gfx::lib::assets {

    std::uint32_t Font::load(const std::string_view path, float size) noexcept {
        const std::uint32_t id = static_cast<std::uint32_t>(storage.size());
        const std::uint32_t atlas = textures.load(path);
        Block block{};
        block.atlas_id = atlas;
        storage.push_back(std::move(block));
        return id;
    }

}