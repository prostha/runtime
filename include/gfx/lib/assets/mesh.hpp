#pragma once
#include <vector>
#include <cstdint>

namespace core::gfx::lib::assets {

    class Mesh {
    public:
        struct Block {
            void* allocation{nullptr};
            std::size_t size{0};
        };

        Mesh() = default;
        ~Mesh() = default;

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&&) noexcept = default;
        Mesh& operator=(Mesh&&) noexcept = default;

        std::uint32_t create(const float* vertices, std::size_t size) noexcept;
        void update(std::uint32_t id, const float* vertices, std::size_t size) noexcept;

        [[nodiscard]] const Block* get(const std::uint32_t id) const noexcept {
            return id < storage.size() ? &storage[id] : nullptr;
        }

        void dispose(const std::uint32_t id) noexcept {
            if (id < storage.size()) storage[id] = {};
        }

        void clear() noexcept {
            storage.clear();
        }

    private:
        std::vector<Block> storage;
    };

}