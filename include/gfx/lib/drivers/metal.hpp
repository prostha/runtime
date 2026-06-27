#pragma once

#include "device.hpp"

namespace core::gfx::lib::drivers {

    struct Context;

    class Metal : public Device {
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

        void shader(std::uint32_t id, const Shader& vertex, const Shader& pixel) noexcept override;

        void mesh(std::uint32_t id, const float* vertices, std::size_t size) noexcept override;
        void update(std::uint32_t id, const float* vertices, std::size_t size) noexcept override;
        void free(std::uint32_t id) noexcept override;

        void texture(std::uint32_t id, const std::uint8_t* pixels, std::uint32_t width, std::uint32_t height) noexcept override;
        void unload(std::uint32_t id) noexcept override;

        void surface(std::uint32_t id, std::uint32_t width, std::uint32_t height) noexcept override;
        void remove(std::uint32_t id) noexcept override;

    private:
        void release() const noexcept;

        Context* context{nullptr};
    };

}