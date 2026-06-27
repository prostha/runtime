#pragma once

#include <unordered_map>
#include <vector>

#include "gfx/lib/drivers/payload.hpp"
#include "shader.hpp"

namespace core::gfx::lib::drivers {

    struct State {
        const float* view{nullptr};
        const float* projection{nullptr};
        Rect region{};
        std::uint32_t active{0};
        std::uint32_t flags{0};
        std::uint32_t pipeline{0};
        const std::unordered_map<std::uint32_t, std::vector<std::uint8_t>>* uniforms{nullptr};
    };

    class Device {
    public:
        virtual ~Device() = default;

        virtual void begin(std::uint32_t width, std::uint32_t height, const State& state) noexcept = 0;
        virtual void submit(const Buffer& stream) noexcept = 0;
        virtual void end() noexcept = 0;
        virtual void dispose() noexcept = 0;

        virtual void shader(std::uint32_t id, const Shader& vertex, const Shader& pixel) noexcept = 0;

        virtual void mesh(std::uint32_t id, const float* vertices, std::size_t size) noexcept = 0;
        virtual void update(std::uint32_t id, const float* vertices, std::size_t size) noexcept = 0;
        virtual void free(std::uint32_t id) noexcept = 0;

        virtual void texture(std::uint32_t id, const std::uint8_t* pixels, std::uint32_t width, std::uint32_t height) noexcept = 0;
        virtual void unload(std::uint32_t id) noexcept = 0;

        virtual void surface(std::uint32_t id, std::uint32_t width, std::uint32_t height) noexcept = 0;
        virtual void remove(std::uint32_t id) noexcept = 0;
    };

}