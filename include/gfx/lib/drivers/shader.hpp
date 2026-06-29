#pragma once

#include <string_view>
#include <vector>
#include <string>
#include <cstdint>

namespace core::gfx::lib::assets {

    class Shader final {
    public:
        struct Block {
            std::string vertex;
            std::string path;
        };

        Shader() noexcept = default;
        ~Shader() noexcept = default;

        std::uint32_t load(std::string_view vertex, std::string_view path) noexcept;
        [[nodiscard]] const Block* get(std::uint32_t id) const noexcept;
        void dispose(std::uint32_t id) noexcept;
        void clear() noexcept;

    private:
        std::vector<Block> storage;
    };

}