#include "gfx/lib/assets/mesh.hpp"

namespace core::gfx::lib::assets {

    std::uint32_t Mesh::create(const float* vertices, const std::size_t size) noexcept {
        storage.push_back({vertices, size});
        return static_cast<std::uint32_t>(storage.size());
    }

    void Mesh::update(const std::uint32_t id, const float* vertices, const std::size_t size) noexcept {
        if (id > 0 && id <= storage.size()) {
            storage[id - 1] = {vertices, size};
        }
    }

    const Mesh::Block* Mesh::get(const std::uint32_t id) const noexcept {
        return (id > 0 && id <= storage.size()) ? &storage[id - 1] : nullptr;
    }

    void Mesh::dispose(const std::uint32_t id) noexcept {
        if (id > 0 && id <= storage.size()) {
            storage[id - 1] = {};
        }
    }

    void Mesh::clear() noexcept {
        storage.clear();
    }

}