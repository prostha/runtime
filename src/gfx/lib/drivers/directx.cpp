#include "gfx/lib/drivers/directx.hpp"

namespace core::gfx::lib::drivers {

    DirectX::DirectX(const void* target) noexcept : handle(const_cast<void*>(target)) {}

    void DirectX::begin(const std::uint32_t width, const std::uint32_t height) noexcept {
        (void)width;
        (void)height;
    }

    void DirectX::submit(const Buffer& stream) noexcept {
        for (std::size_t i = 0; i < stream.count; ++i) {
            const Key key = stream.keys[i];
            const Command& command = stream.commands[i];
            (void)key;
            (void)command;
        }
    }

    void DirectX::end() noexcept {}

    void DirectX::dispose() noexcept {
        handle = nullptr;
    }

}