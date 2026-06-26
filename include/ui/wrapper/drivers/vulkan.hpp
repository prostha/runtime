#pragma once
#include <cstdint>
#include <cstddef>

namespace core::ui::drivers {

    class Vulkan final {
    public:
        explicit Vulkan(const void* target) noexcept;
        ~Vulkan() noexcept = default;

        void begin(std::uint32_t width, std::uint32_t height) const noexcept;
        void draw(const void* source, std::size_t count, const std::uint32_t* keys, std::size_t items) const noexcept;
        void end() const noexcept;
        void dispose() noexcept;

    private:
        void* handle{nullptr};
    };

}