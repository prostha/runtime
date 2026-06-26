#pragma once
#include "payload.hpp"
#include <cstdint>

namespace core::gfx::lib::drivers {

    class Device {
    public:
        virtual ~Device() = default;

        virtual void begin(std::uint32_t width, std::uint32_t height) noexcept = 0;
        virtual void submit(const Buffer& stream) noexcept = 0;
        virtual void end() noexcept = 0;
        virtual void dispose() noexcept = 0;
    };

}