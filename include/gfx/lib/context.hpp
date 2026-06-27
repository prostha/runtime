#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cstddef>

#include "queue.hpp"
#include "assets/font.hpp"
#include "assets/mesh.hpp"
#include "assets/shader.hpp"
#include "drivers/device.hpp"

namespace core::gfx::lib {

enum class Api : std::uint32_t {
    Vulkan = 1,
    Metal = 2,
    DirectX = 3
};

class Context final {
public:
    Context(Api api, const void* target) noexcept;
    ~Context() noexcept = default;

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) noexcept = default;
    Context& operator=(Context&&) noexcept = delete;

    void resize(std::uint32_t width, std::uint32_t height) noexcept;
    void push(drivers::Key key, const drivers::Command& command) noexcept;
    void render() noexcept;
    void flush() noexcept;

    void camera(const float* view, const float* projection) noexcept;
    void viewport(const drivers::Rect& bounds) noexcept;

    std::uint32_t target(std::uint32_t width, std::uint32_t height) const noexcept;
    void bind(std::uint32_t id) noexcept;
    void dispose(std::uint32_t id) noexcept;

    void flags(std::uint32_t state) noexcept;
    void program(std::uint32_t id) noexcept;
    void uniform(std::uint32_t binding, const void* data, std::size_t size) noexcept;

    std::unique_ptr<drivers::Device> device;
    assets::Texture textures;
    assets::Mesh meshes;
    assets::Font fonts{textures};
    assets::Shader shaders;

private:
    Queue queue;

    std::uint32_t width{0};
    std::uint32_t height{0};

    float view[16]{};
    float projection[16]{};
    drivers::Rect region;

    std::vector<drivers::Rect> frames;
    std::uint32_t active{0};

    std::uint32_t state{0};
    std::uint32_t pipeline{0};
    std::unordered_map<std::uint32_t, std::vector<std::uint8_t>> uniforms;
};

}

struct Gfx {
    struct Core {
        void* (*init)(std::uint32_t api, const void* target);
        void (*resize)(void* handle, std::uint32_t width, std::uint32_t height);
        void (*push)(void* handle, std::uint64_t key, const void* command);
        void (*render)(void* handle);
        void (*clear)(void* handle);
    } core;

    struct State {
        void (*camera)(void* handle, const float* view, const float* projection);
        void (*viewport)(void* handle, float x, float y, float width, float height);
        void (*flags)(void* handle, std::uint32_t state);
        void (*program)(void* handle, std::uint32_t id);
        void (*uniform)(void* handle, std::uint32_t binding, const void* data, std::size_t size);
    } state;

    struct Texture {
        std::uint32_t (*load)(void* handle, const char* path);
        std::uint32_t (*create)(void* handle, const std::uint8_t* pixels, std::uint32_t width, std::uint32_t height);
        void (*update)(void* handle, std::uint32_t id, const std::uint8_t* pixels, std::uint32_t width, std::uint32_t height);
        void (*dispose)(void* handle, std::uint32_t id);
    } texture;

    struct Mesh {
        std::uint32_t (*create)(void* handle, const float* vertices, std::size_t size);
        void (*update)(void* handle, std::uint32_t id, const float* vertices, std::size_t size);
        void (*dispose)(void* handle, std::uint32_t id);
    } mesh;

    struct Font {
        std::uint32_t (*load)(void* handle, const char* path);
        void (*dispose)(void* handle, std::uint32_t id);
    } font;

    struct Shader {
        std::uint32_t (*load)(void* handle, const char* vertex, const char* path);
        void (*dispose)(void* handle, std::uint32_t id);
    } shader;

    struct Target {
        std::uint32_t (*create)(void* handle, std::uint32_t width, std::uint32_t height);
        void (*bind)(void* handle, std::uint32_t id);
        void (*dispose)(void* handle, std::uint32_t id);
    } target;
};

#if defined(_WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
    EXPORT const Gfx* gfx() noexcept;
}