#include "gfx/lib/context.hpp"
#include "gfx/lib/drivers/vulkan.hpp"
#include "gfx/lib/drivers/metal.hpp"
#include "gfx/lib/drivers/directx.hpp"

namespace core::gfx::lib {

    Context::Context(const Api api, const void* target) noexcept {
        if (api == Api::Vulkan) {
            this->device = std::make_unique<Vulkan>(target);
        }
        #if defined(__APPLE__)
        else if (api == Api::Metal) {
            this->device = std::make_unique<Metal>(target);
        }
        #endif
        #if defined(_WIN32)
        else if (api == Api::DirectX) {
            this->device = std::make_unique<DirectX>(target);
        }
        #endif
    }

void Context::resize(const std::uint32_t width, const std::uint32_t height) noexcept {
    this->width = width;
    this->height = height;
}

void Context::push(const Key key, const Command& command) noexcept {
    this->queue.push(key, command);
}

void Context::render() noexcept {
    if (this->device) {
        State layout{};
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
        for (int i = 0; i < 16; ++i) this->view[i] = view[i];
    }
    if (projection) {
        for (int i = 0; i < 16; ++i) this->projection[i] = projection[i];
    }
}

void Context::viewport(const Rect& bounds) noexcept {
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
        this->device->remove(id);
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

void gfx_push(void* handle, const std::uint64_t key, const Command* command) {
    if (handle && command) {
        Key internal{};
        internal.value = key;
        static_cast<core::gfx::lib::Context*>(handle)->push(internal, *command);
    }
}

void gfx_render(void* handle) {
    if (handle) {
        static_cast<core::gfx::lib::Context*>(handle)->render();
    }
}

void gfx_clear(void* handle) {
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
        const Rect bounds{x, y, width, height};
        static_cast<core::gfx::lib::Context*>(handle)->viewport(bounds);
    }
}

std::uint32_t gfx_texture(void* handle, const char* path) {
    if (handle && path) {
        auto* context = static_cast<core::gfx::lib::Context*>(handle);
        const std::uint32_t id = context->textures.load(path);
        if (context->device) {
            if (const auto* block = context->textures.get(id)) {
                context->device->texture(id, static_cast<const std::uint8_t*>(block->allocation), block->width, block->height);
            }
        }
        return id;
    }
    return 0;
}

std::uint32_t gfx_pixels(void* handle, const std::uint8_t* pixels, const std::uint32_t width, const std::uint32_t height) {
    if (handle) {
        auto* context = static_cast<core::gfx::lib::Context*>(handle);
        const std::uint32_t id = context->textures.create(pixels, width, height);
        if (context->device) {
            context->device->texture(id, pixels, width, height);
        }
        return id;
    }
    return 0;
}

void gfx_unload(void* handle, const std::uint32_t id) {
    if (handle) {
        auto* context = static_cast<core::gfx::lib::Context*>(handle);
        context->textures.dispose(id);
        if (context->device) {
            context->device->unload(id);
        }
    }
}

std::uint32_t gfx_mesh(void* handle, const float* vertices, const std::size_t size) {
    if (handle) {
        auto* context = static_cast<core::gfx::lib::Context*>(handle);
        const std::uint32_t id = context->meshes.create(vertices, size);
        if (context->device) {
            context->device->mesh(id, vertices, size);
        }
        return id;
    }
    return 0;
}

void gfx_update(void* handle, const std::uint32_t id, const float* vertices, const std::size_t size) {
    if (handle) {
        auto* context = static_cast<core::gfx::lib::Context*>(handle);
        context->meshes.update(id, vertices, size);
        if (context->device) {
            context->device->update(id, vertices, size);
        }
    }
}

void gfx_free(void* handle, const std::uint32_t id) {
    if (handle) {
        auto* context = static_cast<core::gfx::lib::Context*>(handle);
        context->meshes.dispose(id);
        if (context->device) {
            context->device->free(id);
        }
    }
}

std::uint32_t gfx_font(void* handle, const char* path) {
    if (handle && path) {
        return static_cast<core::gfx::lib::Context*>(handle)->fonts.load(path);
    }
    return 0;
}

void gfx_destroy(void* handle, const std::uint32_t id) {
    if (handle) {
        static_cast<core::gfx::lib::Context*>(handle)->fonts.dispose(id);
    }
}

std::uint32_t gfx_target(void* handle, const std::uint32_t width, const std::uint32_t height) {
    if (handle) {
        return static_cast<core::gfx::lib::Context*>(handle)->target(width, height);
    }
    return 0;
}

void gfx_bind(void* handle, const std::uint32_t id) {
    if (handle) {
        static_cast<core::gfx::lib::Context*>(handle)->bind(id);
    }
}

void gfx_delete(void* handle, const std::uint32_t id) {
    if (handle) {
        static_cast<core::gfx::lib::Context*>(handle)->dispose(id);
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

}