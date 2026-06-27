#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include "queue.hpp"
#include "assets/font.hpp"
#include "assets/mesh.hpp"
#include "drivers/device.hpp"

using namespace core::gfx::lib::drivers;
using namespace core::gfx::lib::assets;

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
    void push(Key key, const Command& command) noexcept;
    void render() noexcept;
    void flush() noexcept;

    void camera(const float* view, const float* projection) noexcept;
    void viewport(const Rect& bounds) noexcept;

    std::uint32_t target(std::uint32_t width, std::uint32_t height) const noexcept;
    void bind(std::uint32_t id) noexcept;
    void dispose(std::uint32_t id) noexcept;

    void flags(std::uint32_t state) noexcept;
    void program(std::uint32_t id) noexcept;
    void uniform(std::uint32_t binding, const void* data, std::size_t size) noexcept;

    std::unique_ptr<Device> device;
    Texture textures;
    Mesh meshes;
    Font fonts{textures};

private:
    Queue queue;

    std::uint32_t width{0};
    std::uint32_t height{0};

    float view[16]{};
    float projection[16]{};
    Rect region;

    std::vector<Rect> frames;
    std::uint32_t active{0};

    std::uint32_t state{0};
    std::uint32_t pipeline{0};
    std::unordered_map<std::uint32_t, std::vector<std::uint8_t>> uniforms;
};

}

#if defined(_WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
    EXPORT void* gfx_init(std::uint32_t api, const void* target);
    EXPORT void gfx_resize(void* handle, std::uint32_t width, std::uint32_t height);
    EXPORT void gfx_push(void* handle, std::uint64_t key, const Command* command);
    EXPORT void gfx_render(void* handle);
    EXPORT void gfx_clear(void* handle);

    EXPORT void gfx_camera(void* handle, const float* view, const float* projection);
    EXPORT void gfx_viewport(void* handle, float x, float y, float width, float height);

    EXPORT std::uint32_t gfx_texture(void* handle, const char* path);
    EXPORT std::uint32_t gfx_pixels(void* handle, const std::uint8_t* pixels, std::uint32_t width, std::uint32_t height);
    EXPORT void gfx_unload(void* handle, std::uint32_t id);

    EXPORT std::uint32_t gfx_mesh(void* handle, const float* vertices, std::size_t size);
    EXPORT void gfx_update(void* handle, std::uint32_t id, const float* vertices, std::size_t size);
    EXPORT void gfx_free(void* handle, std::uint32_t id);

    EXPORT std::uint32_t gfx_font(void* handle, const char* path);
    EXPORT void gfx_destroy(void* handle, std::uint32_t id);

    EXPORT std::uint32_t gfx_target(void* handle, std::uint32_t width, std::uint32_t height);
    EXPORT void gfx_bind(void* handle, std::uint32_t id);
    EXPORT void gfx_delete(void* handle, std::uint32_t id);

    EXPORT void gfx_flags(void* handle, std::uint32_t state);
    EXPORT void gfx_program(void* handle, std::uint32_t id);
    EXPORT void gfx_uniform(void* handle, std::uint32_t binding, const void* data, std::size_t size);
}