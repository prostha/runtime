#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <unordered_map>
#include <cstring>
#include <vector>

#include "gfx/lib/drivers/directx.hpp"

#pragma comment(lib, "d3dcompiler.lib")

namespace core::gfx::lib::drivers {

    struct Allocation {
        Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
        D3D12_VERTEX_BUFFER_VIEW view{};
    };

    struct Entry {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        D3D12_GPU_DESCRIPTOR_HANDLE handle{};
        std::uint32_t index{};
    };

    struct Surface {
        Microsoft::WRL::ComPtr<ID3D12Resource> surface;
        D3D12_CPU_DESCRIPTOR_HANDLE handle{};
    };

    struct Context {
        Microsoft::WRL::ComPtr<ID3D12Device> device{nullptr};
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue{nullptr};
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator{nullptr};
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> list{nullptr};
        Microsoft::WRL::ComPtr<IDXGISwapChain3> swapchain{nullptr};
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap{nullptr};
        Microsoft::WRL::ComPtr<ID3D12Fence> fence{nullptr};

        HANDLE event{nullptr};
        std::uint64_t counter{0};
        void* target{nullptr};
        std::uint32_t width{0};
        std::uint32_t height{0};

        std::unordered_map<std::uint32_t, Allocation> buffers;
        std::unordered_map<std::uint32_t, Entry> resources;
        std::unordered_map<std::uint32_t, Surface> surfaces;
        std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> frames;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> signature;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> textures;
        std::unordered_map<std::uint16_t, Microsoft::WRL::ComPtr<ID3D12PipelineState>> pipelines;
    };

    DirectX::DirectX(const void* target) noexcept {
        this->context = new Context();
        this->context->target = const_cast<void*>(target);

        if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&this->context->device)))) {
            return;
        }

        D3D12_COMMAND_QUEUE_DESC queue_desc{};
        queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        if (FAILED(this->context->device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&this->context->queue)))) {
            return;
        }

        if (FAILED(this->context->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->context->allocator)))) {
            return;
        }

        if (FAILED(this->context->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->context->allocator.Get(), nullptr, IID_PPV_ARGS(&this->context->list)))) {
            return;
        }
        this->context->list->Close();

        Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
            return;
        }

        DXGI_SWAP_CHAIN_DESC1 swap{};
        swap.BufferCount = 2;
        swap.Width = 0;
        swap.Height = 0;
        swap.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swap.SampleDesc.Count = 1;

        Microsoft::WRL::ComPtr<IDXGISwapChain1> chain;
        if (const auto window = static_cast<HWND>(this->context->target); FAILED(factory->CreateSwapChainForHwnd(this->context->queue.Get(), window, &swap, nullptr, nullptr, &chain))) {
            return;
        }
        chain.As(&this->context->swapchain);

        D3D12_DESCRIPTOR_HEAP_DESC config{};
        config.NumDescriptors = 16;
        config.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        config.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        if (FAILED(this->context->device->CreateDescriptorHeap(&config, IID_PPV_ARGS(&this->context->heap)))) {
            return;
        }

        this->context->frames.resize(2);
        const SIZE_T stride = this->context->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE start = this->context->heap->GetCPUDescriptorHandleForHeapStart();
        for (std::uint32_t i = 0; i < 2; ++i) {
            this->context->swapchain->GetBuffer(i, IID_PPV_ARGS(&this->context->frames[i]));
            this->context->device->CreateRenderTargetView(this->context->frames[i].Get(), nullptr, start);
            start.ptr += stride;
        }

        D3D12_DESCRIPTOR_HEAP_DESC setup{};
        setup.NumDescriptors = 1024;
        setup.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        setup.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        this->context->device->CreateDescriptorHeap(&setup, IID_PPV_ARGS(&this->context->textures));

        D3D12_ROOT_PARAMETER parameters[5]{};
        parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        parameters[0].Constants.ShaderRegister = 0;
        parameters[0].Constants.Num32BitValues = 16;
        parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        parameters[1].Constants.ShaderRegister = 1;
        parameters[1].Constants.Num32BitValues = 16;
        parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        parameters[2].Constants.ShaderRegister = 2;
        parameters[2].Constants.Num32BitValues = 4;
        parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        parameters[3].Constants.ShaderRegister = 3;
        parameters[3].Constants.Num32BitValues = 16;
        parameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_DESCRIPTOR_RANGE range{};
        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range.NumDescriptors = 1;
        range.BaseShaderRegister = 0;
        range.RegisterSpace = 0;
        range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        parameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        parameters[4].DescriptorTable.NumDescriptorRanges = 1;
        parameters[4].DescriptorTable.pDescriptorRanges = &range;
        parameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_STATIC_SAMPLER_DESC sampler{};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_ROOT_SIGNATURE_DESC layout{};
        layout.NumParameters = 5;
        layout.pParameters = parameters;
        layout.NumStaticSamplers = 1;
        layout.pStaticSamplers = &sampler;
        layout.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        Microsoft::WRL::ComPtr<ID3DBlob> blob;
        Microsoft::WRL::ComPtr<ID3DBlob> errors;
        if (SUCCEEDED(D3D12SerializeRootSignature(&layout, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &errors))) {
            this->context->device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&this->context->signature));
        }

        if (FAILED(this->context->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->context->fence)))) {
            return;
        }

        this->context->event = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
    }

    DirectX::~DirectX() {
        this->release();
    }

    void DirectX::begin(const std::uint32_t width, const std::uint32_t height, const State& state) noexcept {
        if (!this->context) return;
        this->context->width = width;
        this->context->height = height;

        this->context->allocator->Reset();
        this->context->list->Reset(this->context->allocator.Get(), nullptr);

        D3D12_VIEWPORT viewport{};
        if (state.region.width > 0.0f && state.region.height > 0.0f) {
            viewport.TopLeftX = state.region.x;
            viewport.TopLeftY = state.region.y;
            viewport.Width = state.region.width;
            viewport.Height = state.region.height;
        } else {
            viewport.TopLeftX = 0.0f;
            viewport.TopLeftY = 0.0f;
            viewport.Width = static_cast<float>(width);
            viewport.Height = static_cast<float>(height);
        }
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        const D3D12_RECT scissor{
            static_cast<LONG>(viewport.TopLeftX),
            static_cast<LONG>(viewport.TopLeftY),
            static_cast<LONG>(viewport.TopLeftX + viewport.Width),
            static_cast<LONG>(viewport.TopLeftY + viewport.Height)
        };

        this->context->list->RSSetViewports(1, &viewport);
        this->context->list->RSSetScissorRects(1, &scissor);
        this->context->list->SetGraphicsRootSignature(this->context->signature.Get());

        ID3D12DescriptorHeap* heaps[] = { this->context->textures.Get() };
        this->context->list->SetDescriptorHeaps(1, heaps);

        if (state.active == 0) {
            const std::uint32_t index = this->context->swapchain->GetCurrentBackBufferIndex();
            D3D12_RESOURCE_BARRIER barrier{};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = this->context->frames[index].Get();
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
            barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            this->context->list->ResourceBarrier(1, &barrier);

            D3D12_CPU_DESCRIPTOR_HANDLE start = this->context->heap->GetCPUDescriptorHandleForHeapStart();
            const SIZE_T stride = this->context->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            start.ptr += index * stride;
            this->context->list->OMSetRenderTargets(1, &start, FALSE, nullptr);
        } else {
            if (const auto match = this->context->surfaces.find(state.active); match != this->context->surfaces.end()) {
                D3D12_RESOURCE_BARRIER barrier{};
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.pResource = match->second.surface.Get();
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                this->context->list->ResourceBarrier(1, &barrier);
                this->context->list->OMSetRenderTargets(1, &match->second.handle, FALSE, nullptr);
            }
        }

        if (state.view) {
            this->context->list->SetGraphicsRoot32BitConstants(0, 16, state.view, 0);
        }
        if (state.projection) {
            this->context->list->SetGraphicsRoot32BitConstants(1, 16, state.projection, 0);
        }

        if (state.uniforms) {
            for (const auto& [binding, bytes] : *state.uniforms) {
                if (!bytes.empty()) {
                    this->context->list->SetGraphicsRoot32BitConstants(
                        binding + 3,
                        static_cast<UINT>(bytes.size() / 4),
                        bytes.data(),
                        0
                    );
                }
            }
        }
    }

    void DirectX::submit(const Buffer& stream) noexcept {
        if (!this->context || stream.count == 0 || !stream.keys || !stream.commands) {
            return;
        }

        std::uint16_t pipeline = 0xFFFF;
        std::uint32_t texture = 0xFFFFFFFF;

        this->context->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        for (std::size_t i = 0; i < stream.count; ++i) {
            const Key key = stream.keys[i];
            const Command command = stream.commands[i];

            if (key.bits.pipeline != pipeline) {
                pipeline = static_cast<std::uint16_t>(key.bits.pipeline);
                if (const auto match = this->context->pipelines.find(pipeline); match != this->context->pipelines.end()) {
                    this->context->list->SetPipelineState(match->second.Get());
                }
            }

            if (key.bits.texture != texture) {
                texture = static_cast<std::uint32_t>(key.bits.texture);
                if (const auto match = this->context->resources.find(texture); match != this->context->resources.end()) {
                    this->context->list->SetGraphicsRootDescriptorTable(4, match->second.handle);
                }
            }

            if (const auto match = this->context->buffers.find(static_cast<std::uint32_t>(key.bits.entity)); match != this->context->buffers.end()) {
                this->context->list->IASetVertexBuffers(0, 1, &match->second.view);
            }

            this->context->list->SetGraphicsRoot32BitConstants(2, 4, command.box, 0);
            this->context->list->DrawInstanced(4, 1, 0, 0);
        }
    }

    void DirectX::end() noexcept {
        if (!this->context) return;
        const std::uint32_t index = this->context->swapchain->GetCurrentBackBufferIndex();
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = this->context->frames[index].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        this->context->list->ResourceBarrier(1, &barrier);

        this->context->list->Close();
        ID3D12CommandList* tokens[] = {this->context->list.Get()};
        this->context->queue->ExecuteCommandLists(1, tokens);
        this->context->swapchain->Present(1, 0);
        this->sync();
    }

    void DirectX::dispose() noexcept {
        this->release();
    }

    void DirectX::shader(const std::uint32_t id, const Shader& vertex, const Shader& pixel) noexcept {
        if (!this->context || !vertex.code || !pixel.code) return;

        this->sync();

        D3D12_INPUT_ELEMENT_DESC elements[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC state{};
        state.pRootSignature = this->context->signature.Get();
        state.VS = { vertex.code, vertex.size };
        state.PS = { pixel.code, pixel.size };
        state.InputLayout = { elements, 1 };
        state.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        state.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        state.BlendState.RenderTarget[0].BlendEnable = FALSE;
        state.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        state.SampleMask = UINT_MAX;
        state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        state.NumRenderTargets = 1;
        state.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        state.SampleDesc.Count = 1;

        Microsoft::WRL::ComPtr<ID3D12PipelineState> item;
        if (SUCCEEDED(this->context->device->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&item)))) {
            this->context->pipelines[static_cast<std::uint16_t>(id)] = item;
        }
    }

    void DirectX::mesh(const std::uint32_t id, const float* vertices, const std::size_t size) noexcept {
        if (!this->context || !vertices || size == 0) return;

        Allocation item{};
        const UINT bytes = static_cast<UINT>(size * sizeof(float));

        D3D12_HEAP_PROPERTIES props{};
        props.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC config{};
        config.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        config.Width = bytes;
        config.Height = 1;
        config.DepthOrArraySize = 1;
        config.MipLevels = 1;
        config.Format = DXGI_FORMAT_UNKNOWN;
        config.SampleDesc.Count = 1;
        config.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        if (SUCCEEDED(this->context->device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &config, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&item.buffer)))) {
            void* ptr = nullptr;
            if (SUCCEEDED(item.buffer->Map(0, nullptr, &ptr))) {
                std::memcpy(ptr, vertices, bytes);
                item.buffer->Unmap(0, nullptr);
            }
            item.view.BufferLocation = item.buffer->GetGPUVirtualAddress();
            item.view.StrideInBytes = sizeof(float) * 4;
            item.view.SizeInBytes = bytes;
            this->context->buffers[id] = item;
        }
    }

    void DirectX::update(const std::uint32_t id, const float* vertices, const std::size_t size) noexcept {
        if (!this->context) return;
        if (const auto match = this->context->buffers.find(id); match != this->context->buffers.end() && vertices && size > 0) {
            void* ptr = nullptr;
            if (SUCCEEDED(match->second.buffer->Map(0, nullptr, &ptr))) {
                std::memcpy(ptr, vertices, size * sizeof(float));
                match->second.buffer->Unmap(0, nullptr);
            }
        }
    }

    void DirectX::free(const std::uint32_t id) noexcept {
        if (!this->context) return;
        this->sync();
        this->context->buffers.erase(id);
    }

    void DirectX::texture(const std::uint32_t id, const std::uint8_t* pixels, const std::uint32_t width, const std::uint32_t height) noexcept {
        if (!this->context || !pixels || width == 0 || height == 0) return;

        Entry item{};
        item.index = id;

        D3D12_RESOURCE_DESC config{};
        config.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        config.Width = width;
        config.Height = height;
        config.DepthOrArraySize = 1;
        config.MipLevels = 1;
        config.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        config.SampleDesc.Count = 1;
        config.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

        D3D12_HEAP_PROPERTIES heap_props{};
        heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;

        if (SUCCEEDED(this->context->device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &config, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&item.resource)))) {
            UINT pitch = (width * 4 + 255) & ~255;
            UINT bytes = pitch * height;

            D3D12_RESOURCE_DESC setup{};
            setup.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            setup.Width = bytes;
            setup.Height = 1;
            setup.DepthOrArraySize = 1;
            setup.MipLevels = 1;
            setup.Format = DXGI_FORMAT_UNKNOWN;
            setup.SampleDesc.Count = 1;
            setup.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

            D3D12_HEAP_PROPERTIES memory{};
            memory.Type = D3D12_HEAP_TYPE_UPLOAD;

            Microsoft::WRL::ComPtr<ID3D12Resource> upload;
            if (SUCCEEDED(this->context->device->CreateCommittedResource(&memory, D3D12_HEAP_FLAG_NONE, &setup, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload)))) {
                void* ptr = nullptr;
                if (SUCCEEDED(upload->Map(0, nullptr, &ptr))) {
                    for (std::uint32_t y = 0; y < height; ++y) {
                        std::memcpy(static_cast<std::uint8_t*>(ptr) + (y * pitch), pixels + (y * width * 4), width * 4);
                    }
                    upload->Unmap(0, nullptr);
                }

                this->context->allocator->Reset();
                this->context->list->Reset(this->context->allocator.Get(), nullptr);

                D3D12_TEXTURE_COPY_LOCATION target_loc{};
                target_loc.pResource = item.resource.Get();
                target_loc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                target_loc.SubresourceIndex = 0;

                D3D12_TEXTURE_COPY_LOCATION source_loc{};
                source_loc.pResource = upload.Get();
                source_loc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                source_loc.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                source_loc.PlacedFootprint.Footprint.Width = width;
                source_loc.PlacedFootprint.Footprint.Height = height;
                source_loc.PlacedFootprint.Footprint.Depth = 1;
                source_loc.PlacedFootprint.Footprint.RowPitch = pitch;

                this->context->list->CopyTextureRegion(&target_loc, 0, 0, 0, &source_loc, nullptr);

                D3D12_RESOURCE_BARRIER barrier{};
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.pResource = item.resource.Get();
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                this->context->list->ResourceBarrier(1, &barrier);

                this->context->list->Close();
                ID3D12CommandList* tokens[] = {this->context->list.Get()};
                this->context->queue->ExecuteCommandLists(1, tokens);
                this->sync();

                D3D12_SHADER_RESOURCE_VIEW_DESC view{};
                view.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                view.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                view.Texture2D.MipLevels = 1;

                SIZE_T stride = this->context->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                D3D12_CPU_DESCRIPTOR_HANDLE cpu = this->context->textures->GetCPUDescriptorHandleForHeapStart();
                D3D12_GPU_DESCRIPTOR_HANDLE gpu = this->context->textures->GetGPUDescriptorHandleForHeapStart();

                cpu.ptr += id * stride;
                gpu.ptr += id * stride;

                this->context->device->CreateShaderResourceView(item.resource.Get(), &view, cpu);
                item.handle = gpu;
                this->context->resources[id] = item;
            }
        }
    }

    void DirectX::unload(const std::uint32_t id) noexcept {
        if (!this->context) return;
        this->sync();
        this->context->resources.erase(id);
    }

    void DirectX::surface(const std::uint32_t id, const std::uint32_t width, const std::uint32_t height) noexcept {
        if (!this->context) return;

        Surface item{};
        D3D12_RESOURCE_DESC config{};
        config.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        config.Width = width;
        config.Height = height;
        config.DepthOrArraySize = 1;
        config.MipLevels = 1;
        config.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        config.SampleDesc.Count = 1;
        config.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_HEAP_PROPERTIES props{};
        props.Type = D3D12_HEAP_TYPE_DEFAULT;

        if (SUCCEEDED(this->context->device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &config, D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(&item.surface)))) {
            D3D12_CPU_DESCRIPTOR_HANDLE start = this->context->heap->GetCPUDescriptorHandleForHeapStart();
            const SIZE_T stride = this->context->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            start.ptr += (id + 2) * stride;
            item.handle = start;
            this->context->device->CreateRenderTargetView(item.surface.Get(), nullptr, item.handle);
            this->context->surfaces[id] = item;
        }
    }

    void DirectX::remove(const std::uint32_t id) noexcept {
        if (!this->context) return;
        this->sync();
        this->context->surfaces.erase(id);
    }

    void DirectX::release() noexcept {
        if (!this->context) return;
        this->sync();

        for (auto& buffer : this->context->frames) {
            buffer.Reset();
        }
        this->context->frames.clear();

        this->context->buffers.clear();
        this->context->resources.clear();
        this->context->surfaces.clear();
        this->context->pipelines.clear();

        this->context->signature.Reset();
        this->context->textures.Reset();
        if (this->context->event) {
            CloseHandle(this->context->event);
            this->context->event = nullptr;
        }
        this->context->fence.Reset();
        this->context->heap.Reset();
        this->context->swapchain.Reset();
        this->context->list.Reset();
        this->context->allocator.Reset();
        this->context->queue.Reset();
        this->context->device.Reset();

        delete this->context;
        this->context = nullptr;
    }

    void DirectX::sync() const noexcept {
        if (!this->context) return;
        if (const std::uint64_t token = ++this->context->counter; SUCCEEDED(this->context->queue->Signal(this->context->fence.Get(), token))) {
            if (this->context->fence->GetCompletedValue() < token) {
                this->context->fence->SetEventOnCompletion(token, this->context->event);
                WaitForSingleObject(this->context->event, INFINITE);
            }
        }
    }

}