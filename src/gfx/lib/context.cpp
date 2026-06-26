#include "gfx/lib/context.hpp"
#include "gfx/lib/drivers/vulkan.hpp"
#include "gfx/lib/drivers/metal.hpp"
#include "gfx/lib/drivers/directx.hpp"

namespace core::gfx::lib {

    Context::Context(const std::uint32_t api, const void* target) noexcept {
        if (api == 1) {
            device = std::make_unique<Vulkan>(target);
        } else if (api == 2) {
            device = std::make_unique<Metal>(target);
        } else if (api == 3) {
            device = std::make_unique<DirectX>(target);
        }
    }

    void Context::resize(const std::uint32_t width, const std::uint32_t height) noexcept {
        horizontal = width;
        vertical = height;
    }

    void Context::push(const Key key, const Command &command) noexcept {
        queue.push(key, command);
    }

    void Context::render() noexcept {
        if (device) {
            device->begin(horizontal, vertical);
            device->submit(queue.flush());
            device->end();
        }
    }

    void Context::flush() noexcept {
        queue.clear();
    }

}

extern "C" {

    EXPORT void* gfx_init(const std::uint32_t api, const void* target) {
        return new core::gfx::lib::Context(api, target);
    }

    EXPORT void gfx_resize(void* handle, const std::uint32_t width, const std::uint32_t height) {
        if (handle) {
            static_cast<core::gfx::lib::Context*>(handle)->resize(width, height);
        }
    }

    EXPORT void gfx_push(void* handle, const std::uint64_t key, const Command* command) {
        if (handle && command) {
            Key bitkey{};
            bitkey.value = key;
            static_cast<core::gfx::lib::Context*>(handle)->push(bitkey, *command);
        }
    }

    EXPORT void gfx_render(void* handle) {
        if (handle) {
            static_cast<core::gfx::lib::Context*>(handle)->render();
        }
    }

    EXPORT void gfx_clear(void* handle) {
        if (handle) {
            static_cast<core::gfx::lib::Context*>(handle)->flush();
        }
    }

}