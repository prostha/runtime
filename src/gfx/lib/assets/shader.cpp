#include <cstdint>

#include "gfx/lib/assets/shader.hpp"

namespace core::gfx::lib::assets {

    std::uint32_t Shader::load(std::string_view vertex, std::string_view path) noexcept {
        storage.push_back({{vertex.data(), vertex.size()}, {path.data(), path.size()}});
        return static_cast<std::uint32_t>(storage.size());
    }

    const Shader::Block* Shader::get(const std::uint32_t id) const noexcept {
        return (id > 0 && id <= storage.size()) ? &storage[id - 1] : nullptr;
    }

    void Shader::dispose(const std::uint32_t id) noexcept {
        if (id > 0 && id <= storage.size()) {
            storage[id - 1] = {};
        }
    }

    void Shader::clear() noexcept {
        storage.clear();
    }

}