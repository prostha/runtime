#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace core::gfx::lib::assets {

    class Texture {
    public:
        struct Block {
            void* allocation{nullptr};
            std::uint32_t width{0};
            std::uint32_t height{0};
        };

        Texture() = default;
        ~Texture() = default;

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&&) noexcept = default;
        Texture& operator=(Texture&&) noexcept = default;

        std::uint32_t load(std::string_view path) noexcept;
        std::uint32_t create(const std::uint8_t* pixels, std::uint32_t width, std::uint32_t height) noexcept;

        [[nodiscard]] const Block* get(const std::uint32_t id) const noexcept {
            return id < storage.size() ? &storage[id] : nullptr;
        }

        void dispose(const std::uint32_t id) noexcept {
            if (id < storage.size()) storage[id] = {};
        }

        void clear() noexcept {
            registry.clear();
            storage.clear();
        }

    private:
        std::unordered_map<std::string, std::uint32_t> registry;
        std::vector<Block> storage;
    };

}