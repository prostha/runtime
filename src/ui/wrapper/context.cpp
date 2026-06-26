#include "ui/wrapper/context.hpp"
#include "ui/wrapper/render.hpp"
#include "ui/widget.hpp"
#include "core/ecs/world.hpp"
#include <new>

namespace core::ui {

    Context::Context(const Api api, const void* target) noexcept
        : graphics(api, target) {}

    Context::~Context() noexcept {
        valid.store(false, std::memory_order_release);
        graphics.dispose();
    }

    void Context::resize(const std::uint32_t width, const std::uint32_t height) noexcept {
        if (width == 0 || height == 0 || width > 16384 || height > 16384) {
            valid.store(false, std::memory_order_release);
            return;
        }
        horizontal.store(width, std::memory_order_relaxed);
        vertical.store(height, std::memory_order_relaxed);
        valid.store(true, std::memory_order_release);
    }

    void Context::event(const std::uint32_t status, const float x, const float y, const float sx, const float sy) noexcept {
        const std::uint32_t width = horizontal.load(std::memory_order_relaxed);
        const std::uint32_t height = vertical.load(std::memory_order_relaxed);

        if (x < 0.0f || x > static_cast<float>(width) || y < 0.0f || y > static_cast<float>(height)) {
            return;
        }

        state.state = status;
        state.pointer[0] = x;
        state.pointer[1] = y;
        state.scroll[0] = sx;
        state.scroll[1] = sy;
    }

    void Context::render(const void* source, const std::size_t count, const std::uint32_t* keys, const std::size_t items) noexcept {
        if (!valid.load(std::memory_order_acquire)) return;
        if (!source || !keys) return;
        if (count == 0 || items == 0) return;

        const std::uint32_t width = horizontal.load(std::memory_order_relaxed);
        const std::uint32_t height = vertical.load(std::memory_order_relaxed);

        graphics.begin(width, height);
        graphics.draw(source, count, keys, items);
        graphics.end();
    }

    std::uint32_t Context::width() const noexcept {
        return horizontal.load(std::memory_order_relaxed);
    }

    std::uint32_t Context::height() const noexcept {
        return vertical.load(std::memory_order_relaxed);
    }

}

using core::ui::Context;
using core::ui::Api;

extern "C" {
    void* ui_init(std::uint32_t api, const void* target) {
        if (!target) return nullptr;
        return new (std::nothrow) Context(static_cast<Api>(api), target);
    }

    void ui_resize(void* handle, const std::uint32_t width, const std::uint32_t height) {
        if (!handle) return;
        static_cast<Context*>(handle)->resize(width, height);
    }

    void ui_event(void* handle, const std::uint32_t state, const float x, const float y, const float sx, const float sy) {
        if (!handle) return;
        static_cast<Context*>(handle)->event(state, x, y, sx, sy);
    }

    void ui_render(void* handle, const void* source, const std::size_t count, const std::uint32_t* keys, const std::size_t items) {
        if (!handle) return;
        static_cast<Context*>(handle)->render(source, count, keys, items);
    }

    void ui_tick(void* handle, const core::World* world, const std::uint32_t tag) {
        if (!handle || !world) return;

        const auto* context = static_cast<Context*>(handle);
        const auto w = static_cast<float>(context->width());
        const auto h = static_cast<float>(context->height());

        const core::Find query = world->query().with(tag);

        world->loop(query, [&](const std::size_t count, const core::Id*, const std::vector<void*>& tabs) {
            if (tabs.empty() || !tabs[0]) return;

            const auto* widgets = static_cast<core::ui::Widget*>(tabs[0]);
            for (std::size_t i = 0; i < count; ++i) {
                widgets[i].compute(w, h, *world);
            }
        });

        core::ui::dispatch(handle, world, tag);
    }

    void ui_free(void* handle) {
        delete static_cast<Context*>(handle);
    }
}