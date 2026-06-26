#pragma once
#include "drivers/payload.hpp"
#include "drivers/device.hpp"
#include "queue.hpp"
#include <memory>

using namespace core::gfx::lib::drivers;

namespace core::gfx::lib {

    class Context final {
    public:
        Context(std::uint32_t api, const void* target) noexcept;
        ~Context() noexcept = default;

        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;
        Context(Context&&) noexcept = default;
        Context& operator=(Context&&) noexcept = default;

        void resize(std::uint32_t width, std::uint32_t height) noexcept;
        void push(Key key, const Command &command) noexcept;
        void render() noexcept;
        void flush() noexcept;

    private:
        std::unique_ptr<Device> device;
        Queue queue;
        std::uint32_t horizontal{0};
        std::uint32_t vertical{0};
    };

}

#if defined(_WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
    EXPORT void* gfx_init(std::uint32_t api, const void* target);
    EXPORT void gfx_resize(void* handle, std::uint32_t width, std::uint32_t height);
    EXPORT void gfx_push(void* handle, std::uint64_t key, const Command* command);
    EXPORT void gfx_render(void* handle);
    EXPORT void gfx_clear(void* handle);
}