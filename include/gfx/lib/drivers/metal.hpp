#pragma once

#include <cstdint>
#include <cstddef>

#include "device.hpp"

namespace engine {

    class Context;

    class Metal final : public Device {
    public:
        explicit Metal(const void* target) noexcept;
        ~Metal() override;

        Metal(const Metal&) = delete;
        Metal& operator=(const Metal&) = delete;
        Metal(Metal&&) noexcept = default;
        Metal& operator=(Metal&&) noexcept = delete;

        void begin(std::uint32_t width, std::uint32_t height, const State& state) noexcept override;
        void submit(const Buffer& stream) noexcept override;
        void end() noexcept override;
        void dispose() noexcept override;

        void shader(std::uint32_t handle, const Shader& vertex, const Shader& pixel) noexcept override;

        void mesh(std::uint32_t handle, const float* data, std::size_t size) noexcept override;
        void update(std::uint32_t handle, const float* data, std::size_t size) noexcept override;
        void free(std::uint32_t handle) noexcept override;

        void texture(std::uint32_t handle, const std::uint8_t* pixels, std::uint32_t width, std::uint32_t height) noexcept override;
        void unload(std::uint32_t handle) noexcept override;

        void surface(std::uint32_t handle, std::uint32_t width, std::uint32_t height) noexcept override;
        void remove(std::uint32_t handle) noexcept override;

    private:
        void release() const noexcept;

        Context* context;
    };

}