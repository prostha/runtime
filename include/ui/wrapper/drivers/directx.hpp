#pragma once
#include <cstdint>
#include <cstddef>

namespace core::ui::drivers {

    class DirectX final {
    public:
        explicit DirectX(const void* target) noexcept;
        ~DirectX() noexcept = default;

        void begin(std::uint32_t width, std::uint32_t height) const noexcept;
        void draw(const void* source, std::size_t count, const std::uint32_t* keys, std::size_t items) const noexcept;
        void end() const noexcept;
        void dispose() noexcept;

    private:
        void* handle{nullptr};
    };

}