#pragma once
#include <cstdint>
#include <cstddef>
#include <atomic>
#include "graphics.hpp"
#include "input.hpp"

namespace core {
    class World;
}

namespace core::ui {

    class Context final {
    public:
        explicit Context(Api api, const void* target) noexcept;
        ~Context() noexcept;

        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;
        Context(Context&&) noexcept = delete;
        Context& operator=(Context&&) noexcept = delete;

        void resize(std::uint32_t width, std::uint32_t height) noexcept;
        void event(std::uint32_t status, float x, float y, float sx, float sy) noexcept;
        void render(const void* source, std::size_t count, const std::uint32_t* keys, std::size_t items) noexcept;

        [[nodiscard]] std::uint32_t width() const noexcept;
        [[nodiscard]] std::uint32_t height() const noexcept;

    private:
        Graphics graphics;
        Input state{};
        std::atomic<std::uint32_t> horizontal{0};
        std::atomic<std::uint32_t> vertical{0};
        std::atomic<bool> valid{true};
    };

}

#if defined(_WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
    EXPORT void* ui_init(std::uint32_t api, const void* target);
    EXPORT void ui_resize(void* handle, std::uint32_t width, std::uint32_t height);
    EXPORT void ui_event(void* handle, std::uint32_t state, float x, float y, float sx, float sy);
    EXPORT void ui_render(void* handle, const void* source, std::size_t count, const std::uint32_t* keys, std::size_t items);
    EXPORT void ui_tick(void* handle, const core::World* world, std::uint32_t tag);
    EXPORT void ui_free(void* handle);
}