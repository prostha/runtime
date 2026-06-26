#pragma once
#include <variant>
#include <cstdint>
#include "drivers/vulkan.hpp"

#ifdef _WIN32
#include "drivers/directx.hpp"
#endif

#ifdef __APPLE__
#include "drivers/metal.hpp"
#endif

namespace core::ui {

    enum class Api : std::uint32_t {
        Vulkan = 0,
        DirectX3D = 1,
        Metal = 2
    };

    class Graphics final {
    public:
        explicit Graphics(Api api, const void* target) noexcept;
        ~Graphics() noexcept = default;

        Graphics(const Graphics&) = delete;
        Graphics& operator=(const Graphics&) = delete;
        Graphics(Graphics&&) noexcept = default;
        Graphics& operator=(Graphics&&) noexcept = default;

        void begin(std::uint32_t width, std::uint32_t height) noexcept;
        void draw(const void* source, std::size_t count, const std::uint32_t* keys, std::size_t items) noexcept;
        void end() noexcept;
        void dispose() noexcept;

    private:
        std::variant<drivers::Vulkan
        #ifdef _WIN32
            , drivers::DirectX
        #endif
        #ifdef __APPLE__
            , drivers::Metal
        #endif
        > drivers;
    };

}