#include "gfx/lib/context.hpp"
#include "gfx/lib/drivers/vulkan.hpp"
#include <algorithm>
#include <cstring>

#if defined(__APPLE__)
#include "drivers/metal.hpp"
#endif

#if defined(_WIN32)
#include "gfx/lib/drivers/directx.hpp"
#endif

template <typename T, typename T::type M>
struct Accessor {
    friend T::type get(T) { return M; }
};

struct Img { typedef core::gfx::lib::assets::Texture core::gfx::lib::Context::*type; };
template struct Accessor<Img, &core::gfx::lib::Context::images>;
core::gfx::lib::assets::Texture core::gfx::lib::Context::*get(Img);

struct Geo { typedef core::gfx::lib::assets::Mesh core::gfx::lib::Context::*type; };
template struct Accessor<Geo, &core::gfx::lib::Context::geometry>;
core::gfx::lib::assets::Mesh core::gfx::lib::Context::*get(Geo);

struct Fnt { typedef core::gfx::lib::assets::Font core::gfx::lib::Context::*type; };
template struct Accessor<Fnt, &core::gfx::lib::Context::letters>;
core::gfx::lib::assets::Font core::gfx::lib::Context::*get(Fnt);

struct Sdr { typedef core::gfx::lib::assets::Shader core::gfx::lib::Context::*type; };
template struct Accessor<Sdr, &core::gfx::lib::Context::programs>;
core::gfx::lib::assets::Shader core::gfx::lib::Context::*get(Sdr);

namespace core::gfx::lib {

    Context::Context(const Api api, const void* target) noexcept
        : letters(images) {
        if (api == Api::Vulkan) {
            this->device = std::make_unique<drivers::Vulkan>(target);
        }
        #if defined(__APPLE__)
        else if (api == Api::Metal) {
            this->device = std::make_unique<drivers::Metal>(target);
        }
        #endif
        #if defined(_WIN32)
        else if (api == Api::DirectX) {
            this->device = std::make_unique<drivers::DirectX>(target);
        }
        #endif
    }

    void Context::resize(const std::uint32_t width, const std::uint32_t height) noexcept {
        this->width = width;
        this->height = height;
    }

    void Context::push(const drivers::Key& key, const drivers::Command& command) noexcept {
        this->queue.push(key, command);
    }

    void Context::render() noexcept {
        if (this->device) {
            drivers::State state_struct{};
            state_struct.view = this->matrices.empty() ? nullptr : &this->matrices[0];
            state_struct.projection = this->matrices.size() < 32 ? nullptr : &this->matrices[16];
            state_struct.region = this->area;
            state_struct.active = this->active;
            state_struct.flags = this->settings;
            state_struct.pipeline = this->pipeline;
            state_struct.uniforms = &this->uniforms;

            this->device->begin(this->width, this->height, state_struct);
            this->device->submit(this->queue.flush());
            this->device->end();
        }
    }

    void Context::flush() noexcept {
        this->queue.clear();
    }

    void Context::camera(const float* view, const float* projection) noexcept {
        this->matrices.resize(32);
        if (view) std::copy_n(view, 16, this->matrices.begin());
        if (projection) std::copy_n(projection, 16, this->matrices.begin() + 16);
    }

    void Context::viewport(const drivers::Rect& bounds) noexcept {
        this->area = bounds;
    }

    std::uint32_t Context::target(const std::uint32_t width, const std::uint32_t height) noexcept {
        this->targets.push_back({0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)});
        const auto id = static_cast<std::uint32_t>(this->targets.size());
        if (this->device) {
            this->device->surface(id, width, height);
        }
        return id;
    }

    void Context::bind(const std::uint32_t id) noexcept {
        this->active = id;
    }

    void Context::dispose(const std::uint32_t id) noexcept {
        if (id > 0 && id <= this->targets.size()) {
            this->targets[id - 1] = {};
            if (this->device) {
                this->device->unload(id);
            }
        }
    }

    void Context::flags(const std::uint32_t setup) noexcept {
        this->settings = setup;
    }

    void Context::program(const std::uint32_t asset) noexcept {
        this->pipeline = asset;
        if (this->device) {
            if (this->programs.get(asset)) {
                this->device->shader(asset, this->programs, this->programs);
            }
        }
    }

    void Context::uniform(const std::uint32_t slot, const void* data, const std::size_t size) noexcept {
        if (data && size > 0) {
            auto& buffer = this->uniforms[slot];
            buffer.resize(size);
            std::memcpy(buffer.data(), data, size);
        }
    }

    static void* binding_core_init(std::uint32_t api, const void* target) {
        return new Context(static_cast<Api>(api), target);
    }

    static void binding_core_resize(void* handle, std::uint32_t width, std::uint32_t height) {
        if (handle) static_cast<Context*>(handle)->resize(width, height);
    }

    static void binding_core_push(void* handle, std::uint64_t key, const void* command) {
        if (handle && command) {
            drivers::Key k;
            k.value = key;
            static_cast<Context*>(handle)->push(k, *static_cast<const drivers::Command*>(command));
        }
    }

    static void binding_core_render(void* handle) {
        if (handle) static_cast<Context*>(handle)->render();
    }

    static void binding_core_clear(void* handle) {
        if (handle) static_cast<Context*>(handle)->flush();
    }

    static void binding_state_camera(void* handle, const float* view, const float* projection) {
        if (handle) static_cast<Context*>(handle)->camera(view, projection);
    }

    static void binding_state_viewport(void* handle, float x, float y, float width, float height) {
        if (handle) static_cast<Context*>(handle)->viewport({x, y, width, height});
    }

    static void binding_state_flags(void* handle, std::uint32_t setup) {
        if (handle) static_cast<Context*>(handle)->flags(setup);
    }

    static void binding_state_program(void* handle, std::uint32_t asset) {
        if (handle) static_cast<Context*>(handle)->program(asset);
    }

    static void binding_state_uniform(void* handle, std::uint32_t slot, const void* data, std::size_t size) {
        if (handle) static_cast<Context*>(handle)->uniform(slot, data, size);
    }

    static std::uint32_t binding_texture_load(void* handle, const char* path) {
        return handle && path ? (static_cast<Context*>(handle)->*get(Img())).load(path) : 0;
    }

    static std::uint32_t binding_texture_create(void* handle, const std::uint8_t* pixels, std::uint32_t width, std::uint32_t height) {
        return handle ? (static_cast<Context*>(handle)->*get(Img())).create(pixels, width, height) : 0;
    }

    static void binding_texture_update(void* handle, std::uint32_t id, const std::uint8_t* pixels, std::uint32_t width, std::uint32_t height) {
        if (handle) (static_cast<Context*>(handle)->*get(Img())).update(id, pixels, width, height);
    }

    static void binding_texture_dispose(void* handle, std::uint32_t id) {
        if (handle) (static_cast<Context*>(handle)->*get(Img())).dispose(id);
    }

    static std::uint32_t binding_mesh_create(void* handle, const float* vertices, std::size_t size) {
        return handle ? (static_cast<Context*>(handle)->*get(Geo())).create(vertices, size) : 0;
    }

    static void binding_mesh_update(void* handle, std::uint32_t id, const float* vertices, std::size_t size) {
        if (handle) (static_cast<Context*>(handle)->*get(Geo())).update(id, vertices, size);
    }

    static void binding_mesh_dispose(void* handle, std::uint32_t id) {
        if (handle) (static_cast<Context*>(handle)->*get(Geo())).dispose(id);
    }

    static std::uint32_t binding_font_load(void* handle, const char* path) {
        return handle && path ? (static_cast<Context*>(handle)->*get(Fnt())).load(path) : 0;
    }

    static void binding_font_dispose(void* handle, std::uint32_t id) {
        if (handle) (static_cast<Context*>(handle)->*get(Fnt())).dispose(id);
    }

    static std::uint32_t binding_shader_load(void* handle, const char* vertex, const char* path) {
        return handle && vertex && path ? (static_cast<Context*>(handle)->*get(Sdr())).load(vertex, path) : 0;
    }

    static void binding_shader_dispose(void* handle, std::uint32_t id) {
        if (handle) (static_cast<Context*>(handle)->*get(Sdr())).dispose(id);
    }

    static std::uint32_t binding_target_create(void* handle, std::uint32_t width, std::uint32_t height) {
        return handle ? static_cast<Context*>(handle)->target(width, height) : 0;
    }

    static void binding_target_bind(void* handle, std::uint32_t id) {
        if (handle) static_cast<Context*>(handle)->bind(id);
    }

    static void binding_target_dispose(void* handle, std::uint32_t id) {
        if (handle) static_cast<Context*>(handle)->dispose(id);
    }

}

extern "C" {
    EXPORT const core::gfx::lib::Bindings* bindings() noexcept {
        static const core::gfx::lib::Bindings layout = {
            { core::gfx::lib::binding_core_init, core::gfx::lib::binding_core_resize, core::gfx::lib::binding_core_push, core::gfx::lib::binding_core_render, core::gfx::lib::binding_core_clear },
            { core::gfx::lib::binding_state_camera, core::gfx::lib::binding_state_viewport, core::gfx::lib::binding_state_flags, core::gfx::lib::binding_state_program, core::gfx::lib::binding_state_uniform },
            { core::gfx::lib::binding_texture_load, core::gfx::lib::binding_texture_create, core::gfx::lib::binding_texture_update, core::gfx::lib::binding_texture_dispose },
            { core::gfx::lib::binding_mesh_create, core::gfx::lib::binding_mesh_update, core::gfx::lib::binding_mesh_dispose },
            { core::gfx::lib::binding_font_load, core::gfx::lib::binding_font_dispose },
            { core::gfx::lib::binding_shader_load, core::gfx::lib::binding_shader_dispose },
            { core::gfx::lib::binding_target_create, core::gfx::lib::binding_target_bind, core::gfx::lib::binding_target_dispose }
        };
        return &layout;
    }
}