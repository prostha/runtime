#include "../include/ui/drivers/directx.hpp"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <memory>
#include <array>

namespace core::ui::drivers {

namespace {
    using Microsoft::WRL::ComPtr;

    struct Context {
        ComPtr<IDXGIFactory4> factory;
        ComPtr<IDXGIAdapter1> adapter;
        ComPtr<ID3D12Device> device;
        ComPtr<ID3D12CommandQueue> queue;
        ComPtr<ID3D12CommandAllocator> allocator;
        ComPtr<ID3D12GraphicsCommandList> list;
    };
}

DirectX::DirectX(const void* target) noexcept {
    (void)target;
    auto state = std::make_unique<Context>();

    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&state->factory)))) return;
    if (FAILED(state->factory->EnumAdapters1(0, &state->adapter))) return;
    if (FAILED(D3D12CreateDevice(state->adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&state->device)))) return;

    D3D12_COMMAND_QUEUE_DESC info{};
    info.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    info.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
    info.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    if (FAILED(state->device->CreateCommandQueue(&info, IID_PPV_ARGS(&state->queue)))) return;

    if (FAILED(state->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&state->allocator)))) return;
    if (FAILED(state->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, state->allocator.Get(), nullptr, IID_PPV_ARGS(&state->list)))) return;
    (void)state->list->Close();

    handle = state.release();
}

void DirectX::begin(const std::uint32_t width, const std::uint32_t height) const noexcept {
    auto* state = static_cast<Context*>(handle);
    if (!state) return;
    (void)width;
    (void)height;
    (void)state->allocator->Reset();
    (void)state->list->Reset(state->allocator.Get(), nullptr);
}

void DirectX::draw(const void* source, const std::size_t count, const std::uint32_t* keys, std::size_t items) const noexcept {
    const auto* state = static_cast<Context*>(handle);
    if (!state || !source || !keys) return;

    (void)source;
    (void)count;

    state->list->DrawIndexedInstanced(static_cast<UINT>(items), 1, 0, 0, 0);
}

void DirectX::end() const noexcept {
    const auto* state = static_cast<Context*>(handle);
    if (!state) return;
    (void)state->list->Close();
    const std::array<ID3D12CommandList*, 1> lists{ state->list.Get() };
    state->queue->ExecuteCommandLists(1, lists.data());
}

void DirectX::dispose() noexcept {
    if (handle) {
        delete static_cast<Context*>(handle);
        handle = nullptr;
    }
}

}