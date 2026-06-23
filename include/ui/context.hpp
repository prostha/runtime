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

    class Context final {
    public:
        explicit Context(Api api, const void* target) noexcept;
        ~Context() noexcept = default;

        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;
        Context(Context&&) noexcept = default;
        Context& operator=(Context&&) noexcept = default;

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
        > storage;
    };

}