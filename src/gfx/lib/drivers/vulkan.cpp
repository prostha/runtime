#include "gfx/lib/drivers/vulkan.hpp"

namespace core::gfx::lib::drivers {

    Vulkan::Vulkan(const void* target) noexcept : handle(const_cast<void*>(target)) {}

    void Vulkan::begin(const std::uint32_t width, const std::uint32_t height) noexcept {
        (void)width;
        (void)height;
    }

    void Vulkan::submit(const Buffer& stream) noexcept {
        for (std::size_t i = 0; i < stream.count; ++i) {
            const Key key = stream.keys[i];
            const Command& command = stream.commands[i];
            (void)key;
            (void)command;
        }
    }

    void Vulkan::end() noexcept {}

    void Vulkan::dispose() noexcept {
        handle = nullptr;
    }

}