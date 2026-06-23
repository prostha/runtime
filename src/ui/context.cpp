#include "../include/ui/context.hpp"

namespace core::ui {

    Context::Context(const Api api, const void* target) noexcept
        : storage([&]() -> decltype(storage) {
            switch (api) {
                #ifdef _WIN32
                case Api::DirectX3D:
                    return ui::drivers::DirectX(target);
                #endif
                #ifdef __APPLE__
                case Api::Metal:
                    return ui::drivers::Metal(target);
                #endif
                case Api::Vulkan:
                default:
                    return ui::drivers::Vulkan(target);
            }
        }()) {}

    void Context::begin(std::uint32_t width, std::uint32_t height) noexcept {
        std::visit([=](auto& type) { type.begin(width, height); }, storage);
    }

    void Context::draw(const void* source, std::size_t count, const std::uint32_t* keys, std::size_t items) noexcept {
        std::visit([=](auto& type) { type.draw(source, count, keys, items); }, storage);
    }

    void Context::end() noexcept {
        std::visit([](auto& type) { type.end(); }, storage);
    }

    void Context::dispose() noexcept {
        std::visit([](auto& type) { type.dispose(); }, storage);
    }

}