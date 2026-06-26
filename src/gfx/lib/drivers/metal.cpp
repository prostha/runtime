#include "gfx/lib/drivers/metal.hpp"

namespace core::gfx::lib::drivers {

    Metal::Metal(const void* target) noexcept : handle(const_cast<void*>(target)) {}

    void Metal::begin(const std::uint32_t width, const std::uint32_t height) noexcept {
        (void)width;
        (void)height;
    }

    void Metal::submit(const Buffer& stream) noexcept {
        for (std::size_t i = 0; i < stream.count; ++i) {
            const Key key = stream.keys[i];
            const Command& command = stream.commands[i];
            (void)key;
            (void)command;
        }
    }

    void Metal::end() noexcept {}

    void Metal::dispose() noexcept {
        handle = nullptr;
    }

}