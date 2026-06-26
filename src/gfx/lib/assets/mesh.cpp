#include "gfx/lib/assets/mesh.hpp"

namespace core::gfx::lib::assets {

    std::uint32_t Mesh::create(const float* vertices, const std::size_t size) noexcept {
        const std::uint32_t id = static_cast<std::uint32_t>(storage.size());
        storage.push_back(Block{const_cast<float*>(vertices), size});
        return id;
    }

    void Mesh::update(const std::uint32_t id, const float* vertices, const std::size_t size) noexcept {
        if (id < storage.size()) {
            storage[id].allocation = const_cast<float*>(vertices);
            storage[id].size = size;
        }
    }

}