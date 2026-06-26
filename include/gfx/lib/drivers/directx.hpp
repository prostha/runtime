#pragma once
#include "device.hpp"

namespace core::gfx::lib::drivers {

    class DirectX final : public Device {
    public:
        explicit DirectX(const void* target) noexcept;
        ~DirectX() override = default;

        void begin(std::uint32_t width, std::uint32_t height) noexcept override;
        void submit(const Buffer& stream) noexcept override;
        void end() noexcept override;
        void dispose() noexcept override;

    private:
        void* handle{nullptr};
    };

}