#include "ui/wrapper/graphics.hpp"

namespace core::ui {

    Graphics::Graphics(const Api api, const void* target) noexcept
        : drivers([&]() -> decltype(drivers) {
            #ifdef _WIN32
            if (api == Api::DirectX3D) {
                return drivers::DirectX(target);
            }
            #endif
            #ifdef __APPLE__
            if (api == Api::Metal) {
                return drivers::Metal(target);
            }
            #endif
            return drivers::Vulkan(target);
        }())
    {
    }

    void Graphics::begin(std::uint32_t width, std::uint32_t height) noexcept {
        std::visit([width, height](auto& driver) { driver.begin(width, height); }, drivers);
    }

    void Graphics::draw(const void* source, std::size_t count, const std::uint32_t* keys, std::size_t items) noexcept {
        std::visit([source, count, keys, items](auto& driver) { driver.draw(source, count, keys, items); }, drivers);
    }

    void Graphics::end() noexcept {
        std::visit([](auto& driver) { driver.end(); }, drivers);
    }

    void Graphics::dispose() noexcept {
        std::visit([](auto& driver) { driver.dispose(); }, drivers);
    }

}