#include "gfx/lib/context.hpp"
#include "gfx/lib/drivers/vulkan.hpp"

#if defined(__APPLE__)
#include "drivers/metal.hpp"
#endif

#if defined(_WIN32)
#include "gfx/lib/drivers/directx.hpp"
#endif

namespace core::gfx::lib {

    Context::Context(const Api api, const void* target) noexcept {
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

    void Context::push(const drivers::Key key, const drivers::Command& command) noexcept {
        this->queue.push(key, command);
    }

    void Context::render() noexcept {
        if (this->device) {
            drivers::State layout{};
            layout.view = this->view;
            layout.projection = this->projection;
            layout.region = this->region;
            layout.active = this->active;
            layout.flags = this->state;
            layout.pipeline = this->pipeline;
            layout.uniforms = &this->uniforms;

            this->device->begin(this->width, this->height, layout);
            this->device->submit(this->queue.flush());
            this->device->end();
        }
    }

    void Context::flush() noexcept {
        this->queue.clear();
    }

    void Context::camera(const float* view, const float* projection) noexcept {
        if (view) {
            for (int index = 0; index < 16; ++index) this->view[index] = view[index];
        }
        if (projection) {
            for (int index = 0; index < 16; ++index) this->projection[index] = projection[index];
        }
    }

    void Context::viewport(const drivers::Rect& bounds) noexcept {
        this->region = bounds;
    }

    std::uint32_t Context::target(const std::uint32_t width, const std::uint32_t height) const noexcept {
        static std::uint32_t generator = 1;
        const std::uint32_t id = generator++;
        if (this->device) {
            this->device->surface(id, width, height);
        }
        return id;
    }

    void Context::bind(const std::uint32_t id) noexcept {
        this->active = id;
    }

    void Context::dispose(const std::uint32_t id) noexcept {
        if (this->device) {
            this->device->unload(id);
        }
        if (this->active == id) {
            this->active = 0;
        }
    }

    void Context::flags(const std::uint32_t state) noexcept {
        this->state = state;
    }

    void Context::program(const std::uint32_t id) noexcept {
        this->pipeline = id;
    }

    void Context::uniform(const std::uint32_t binding, const void* data, const std::size_t size) noexcept {
        if (data && size > 0) {
            auto& bytes = this->uniforms[binding];
            bytes.assign(
                static_cast<const std::uint8_t*>(data),
                static_cast<const std::uint8_t*>(data) + size
            );
        }
    }

}

extern "C" {

    void* gfx_init(const std::uint32_t api, const void* target) {
        return new core::gfx::lib::Context(static_cast<core::gfx::lib::Api>(api), target);
    }

    void gfx_resize(void* handle, const std::uint32_t width, const std::uint32_t height) {
        if (handle) {
            static_cast<core::gfx::lib::Context*>(handle)->resize(width, height);
        }
    }

    void gfx_push(void* handle, const std::uint64_t key, const void* command) {
        if (handle && command) {
            core::gfx::lib::drivers::Key signature{};
            signature.value = key;
            static_cast<core::gfx::lib::Context*>(handle)->push(signature, *static_cast<const core::gfx::lib::drivers::Command*>(command));
        }
    }

    void gfx_render(void* handle) {
        if (handle) {
            static_cast<core::gfx::lib::Context*>(handle)->render();
        }
    }

    void gfx_flush(void* handle) {
        if (handle) {
            static_cast<core::gfx::lib::Context*>(handle)->flush();
        }
    }

    void gfx_camera(void* handle, const float* view, const float* projection) {
        if (handle) {
            static_cast<core::gfx::lib::Context*>(handle)->camera(view, projection);
        }
    }

    void gfx_viewport(void* handle, const float x, const float y, const float width, const float height) {
        if (handle) {
            const core::gfx::lib::drivers::Rect bounds{x, y, width, height};
            static_cast<core::gfx::lib::Context*>(handle)->viewport(bounds);
        }
    }

    void gfx_flags(void* handle, const std::uint32_t state) {
        if (handle) {
            static_cast<core::gfx::lib::Context*>(handle)->flags(state);
        }
    }

    void gfx_program(void* handle, const std::uint32_t id) {
        if (handle) {
            static_cast<core::gfx::lib::Context*>(handle)->program(id);
        }
    }

    void gfx_uniform(void* handle, const std::uint32_t binding, const void* data, const std::size_t size) {
        if (handle) {
            static_cast<core::gfx::lib::Context*>(handle)->uniform(binding, data, size);
        }
    }

    std::uint32_t gfx_texture_load(void* handle, const char* path) {
        if (handle && path) {
            auto* context = static_cast<core::gfx::lib::Context*>(handle);
            const auto id = context->textures.load(path);
            if (context->device) {
                if (const auto* block = context->textures.get(id)) {
                    context->device->texture(id, static_cast<const std::uint8_t*>(block->allocation), block->width, block->height);
                }
            }
            return id;
        }
        return 0;
    }

    std::uint32_t gfx_texture_create(void* handle, const std::uint8_t* pixels, std::uint32_t width, std::uint32_t height) {
        if (handle) {
            auto* context = static_cast<core::gfx::lib::Context*>(handle);
            const auto id = context->textures.create(pixels, width, height);
            if (context->device) {
                context->device->texture(id, pixels, width, height);
            }
            return id;
        }
        return 0;
    }

    void gfx_texture_update(void* handle, const std::uint32_t id, const std::uint8_t* pixels, const std::uint32_t width, const std::uint32_t height) {
        if (handle) {
            auto* context = static_cast<core::gfx::lib::Context*>(handle);
            context->textures.update(id, pixels, width, height);
            if (context->device) {
                context->device->texture(id, pixels, width, height);
            }
        }
    }

    void gfx_texture_dispose(void* handle, std::uint32_t id) {
        if (handle) {
            auto* context = static_cast<core::gfx::lib::Context*>(handle);
            context->textures.dispose(id);
            if (context->device) {
                context->device->unload(id);
            }
        }
    }

    std::uint32_t gfx_mesh_create(void* handle, const float* vertices, std::size_t size) {
        if (handle) {
            auto* context = static_cast<core::gfx::lib::Context*>(handle);
            const auto id = context->meshes.create(vertices, size);
            if (context->device) {
                context->device->mesh(id, vertices, size);
            }
            return id;
        }
        return 0;
    }

    void gfx_mesh_update(void* handle, const std::uint32_t id, const float* vertices, const std::size_t size) {
        if (handle) {
            auto* context = static_cast<core::gfx::lib::Context*>(handle);
            context->meshes.update(id, vertices, size);
            if (context->device) {
                context->device->update(id, vertices, size);
            }
        }
    }

    void gfx_mesh_dispose(void* handle, const std::uint32_t id) {
        if (handle) {
            auto* context = static_cast<core::gfx::lib::Context*>(handle);
            context->meshes.dispose(id);
            if (context->device) {
                context->device->free(id);
            }
        }
    }

    std::uint32_t gfx_font_load(void* handle, const char* path) {
        if (handle && path) {
            auto* context = static_cast<core::gfx::lib::Context*>(handle);
            const auto id = context->fonts.load(path);
            if (context->device) {
                if (const auto* block = context->fonts.get(id)) {
                    if (const auto* texture = context->textures.get(block->atlas)) {
                        context->device->texture(block->atlas, static_cast<const std::uint8_t*>(texture->allocation), texture->width, texture->height);
                    }
                }
            }
            return id;
        }
        return 0;
    }

    void gfx_font_dispose(void* handle, const std::uint32_t id) {
        if (handle) {
            auto* context = static_cast<core::gfx::lib::Context*>(handle);
            context->fonts.dispose(id);
        }
    }

    std::uint32_t gfx_shader_load(void* handle, const char* vertex, const char* path) {
        if (handle && vertex && path) {
            auto* context = static_cast<core::gfx::lib::Context*>(handle);
            const auto id = context->shaders.load(vertex, path);
            if (context->device) {
                if (const auto* block = context->shaders.get(id)) {
                    context->device->shader(id, {block->vertex.code, block->vertex.size}, {block->pixel.code, block->pixel.size});
                }
            }
            return id;
        }
        return 0;
    }

    void gfx_shader_dispose(void* handle, std::uint32_t id) {
        if (handle) {
            auto* context = static_cast<core::gfx::lib::Context*>(handle);
            context->shaders.dispose(id);
        }
    }

    std::uint32_t gfx_target_create(void* handle, const std::uint32_t width, const std::uint32_t height) {
        if (handle) {
            return static_cast<core::gfx::lib::Context*>(handle)->target(width, height);
        }
        return 0;
    }

    void gfx_target_bind(void* handle, const std::uint32_t id) {
        if (handle) {
            static_cast<core::gfx::lib::Context*>(handle)->bind(id);
        }
    }

    void gfx_target_dispose(void* handle, const std::uint32_t id) {
        if (handle) {
            static_cast<core::gfx::lib::Context*>(handle)->dispose(id);
        }
    }

    const Gfx* gfx() noexcept {
        static const Gfx instance = {
            { gfx_init, gfx_resize, gfx_push, gfx_render, gfx_flush },
            { gfx_camera, gfx_viewport, gfx_flags, gfx_program, gfx_uniform },
            { gfx_texture_load, gfx_texture_create, gfx_texture_update, gfx_texture_dispose },
            { gfx_mesh_create, gfx_mesh_update, gfx_mesh_dispose },
            { gfx_font_load, gfx_font_dispose },
            { gfx_shader_load, gfx_shader_dispose },
            { gfx_target_create, gfx_target_bind, gfx_target_dispose }
        };
        return &instance;
    }

}