#pragma once
#include "device.hpp"

namespace core::gfx::lib::drivers {

    class Metal final : public Device {
    public:
        explicit Metal(const void* target) noexcept;
        ~Metal() override = default;

        void begin(std::uint32_t width, std::uint32_t height) noexcept override;
        void submit(const Buffer& stream) noexcept override;
        void end() noexcept override;
        void dispose() noexcept override;

    private:
        void* handle{nullptr};
    };

}