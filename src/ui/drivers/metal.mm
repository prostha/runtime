#include "drivers/metal.hpp"
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <memory>

namespace core::ui::drivers {

namespace {
    struct Context {
        id<MTLDevice> device{nil};
        id<MTLCommandQueue> queue{nil};
        CAMetalLayer* layer{nil};
        id<MTLCommandBuffer> buffer{nil};
        id<MTLRenderCommandEncoder> encoder{nil};
        id<CAMetalDrawable> drawable{nil};
    };
}

Metal::Metal(const void* target) noexcept {
    auto state = std::make_unique<Context>();

    state->device = MTLCreateSystemDefaultDevice();
    if (!state->device) return;

    state->queue = [state->device newCommandQueue];
    if (!state->queue) return;

    state->layer = (__bridge CAMetalLayer*)target;
    if (state->layer) {
        state->layer.device = state->device;
        state->layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    }

    handle = state.release();
}

void Metal::begin(std::uint32_t width, std::uint32_t height) const noexcept {
    auto* state = static_cast<Context*>(handle);
    if (!state || !state->layer) return;
    (void)width;
    (void)height;

    state->buffer = [state->queue commandBuffer];
    if (!state->buffer) return;

    state->drawable = [state->layer nextDrawable];
    if (!state->drawable) return;

    MTLRenderPassDescriptor* descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    descriptor.colorAttachments[0].texture = state->drawable.texture;
    descriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    descriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);

    state->encoder = [state->buffer renderCommandEncoderWithDescriptor:descriptor];
}

void Metal::draw(const void* source, std::size_t count, const std::uint32_t* keys, std::size_t items) const noexcept {
    auto* state = static_cast<Context*>(handle);
    if (!state || !state->encoder || !source || !keys) return;

    id<MTLBuffer> inputs = [state->device newBufferWithBytes:source length:count options:MTLResourceStorageModeShared];
    id<MTLBuffer> indices = [state->device newBufferWithBytes:keys length:items * sizeof(std::uint32_t) options:MTLResourceStorageModeShared];

    [state->encoder setVertexBuffer:inputs offset:0 atIndex:0];
    [state->encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:items indexType:MTLIndexTypeUInt32 indexBuffer:indices indexBufferOffset:0];
}

void Metal::end() const noexcept {
    auto* state = static_cast<Context*>(handle);
    if (!state) return;

    if (state->encoder) {
        [state->encoder endEncoding];
        state->encoder = nil;
    }

    if (state->buffer && state->drawable) {
        [state->buffer presentDrawable:state->drawable];
        [state->buffer commit];
    }

    state->drawable = nil;
    state->buffer = nil;
}

void Metal::dispose() noexcept {
    if (handle) {
        auto* state = static_cast<Context*>(handle);
        state->drawable = nil;
        state->encoder = nil;
        state->buffer = nil;
        state->layer = nil;
        state->queue = nil;
        state->device = nil;
        delete state;
        handle = nullptr;
    }
}

}