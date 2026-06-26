#pragma once
#include "device.hpp"

namespace core::gfx::lib::drivers {

    class Vulkan final : public Device {
    public:
        explicit Vulkan(const void* target) noexcept;
        ~Vulkan() override = default;

        void begin(std::uint32_t width, std::uint32_t height) noexcept override;
        void submit(const Buffer& stream) noexcept override;
        void end() noexcept override;
        void dispose() noexcept override;

    private:
        void* handle{nullptr};
    };

}