#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <ranges>


#include "gfx/lib/drivers/vulkan.hpp"

namespace core::gfx::lib::drivers {

    struct Allocation {
        VkBuffer buffer{VK_NULL_HANDLE};
        VkDeviceMemory memory{VK_NULL_HANDLE};
        std::size_t size{0};
    };

    struct Entry {
        VkImage image{VK_NULL_HANDLE};
        VkDeviceMemory memory{VK_NULL_HANDLE};
        VkImageView view{VK_NULL_HANDLE};
        VkDescriptorSet set{VK_NULL_HANDLE};
    };

    struct Surface {
        VkImage image{VK_NULL_HANDLE};
        VkDeviceMemory memory{VK_NULL_HANDLE};
        VkImageView view{VK_NULL_HANDLE};
        VkFramebuffer framebuffer{VK_NULL_HANDLE};
    };

    struct Context {
        void* target{nullptr};
        std::uint32_t width{0};
        std::uint32_t height{0};

        VkInstance instance{VK_NULL_HANDLE};
        VkPhysicalDevice physical{VK_NULL_HANDLE};
        VkDevice device{VK_NULL_HANDLE};
        VkQueue queue{VK_NULL_HANDLE};
        VkCommandPool pool{VK_NULL_HANDLE};
        VkCommandBuffer buffer{VK_NULL_HANDLE};

        VkImage image{VK_NULL_HANDLE};
        VkDeviceMemory memory{VK_NULL_HANDLE};
        VkImageView view{VK_NULL_HANDLE};
        VkRenderPass pass{VK_NULL_HANDLE};
        VkFramebuffer frame{VK_NULL_HANDLE};

        VkPipelineLayout layout{VK_NULL_HANDLE};
        VkDescriptorSetLayout descriptors{VK_NULL_HANDLE};
        VkDescriptorPool assets{VK_NULL_HANDLE};
        VkSampler sampler{VK_NULL_HANDLE};

        std::unordered_map<std::uint32_t, Allocation> buffers;
        std::unordered_map<std::uint32_t, Entry> resources;
        std::unordered_map<std::uint32_t, Surface> surfaces;
        std::unordered_map<std::uint16_t, VkPipeline> pipelines;
    };

    Vulkan::Vulkan(const void* target) noexcept {
        this->context = new Context();
        this->context->target = const_cast<void*>(target);

        VkApplicationInfo metadata{};
        metadata.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        metadata.pApplicationName = "Headless Viewport";
        metadata.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        metadata.pEngineName = "Prostha Rendering Engine";
        metadata.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        metadata.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo setup{};
        setup.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        setup.pApplicationInfo = &metadata;

        if (vkCreateInstance(&setup, nullptr, &this->context->instance) != VK_SUCCESS) {
            return;
        }

        std::uint32_t count = 0;
        vkEnumeratePhysicalDevices(this->context->instance, &count, nullptr);
        if (count == 0) return;
        std::vector<VkPhysicalDevice> list(count);
        vkEnumeratePhysicalDevices(this->context->instance, &count, list.data());
        this->context->physical = list[0];

        constexpr float rate = 1.0f;
        VkDeviceQueueCreateInfo queue{};
        queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue.queueFamilyIndex = 0;
        queue.queueCount = 1;
        queue.pQueuePriorities = &rate;

        VkDeviceCreateInfo device{};
        device.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device.queueCreateInfoCount = 1;
        device.pQueueCreateInfos = &queue;

        if (vkCreateDevice(this->context->physical, &device, nullptr, &this->context->device) != VK_SUCCESS) {
            return;
        }

        vkGetDeviceQueue(this->context->device, 0, 0, &this->context->queue);

        VkCommandPoolCreateInfo pool{};
        pool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool.queueFamilyIndex = 0;
        pool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        vkCreateCommandPool(this->context->device, &pool, nullptr, &this->context->pool);

        VkCommandBufferAllocateInfo grant{};
        grant.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        grant.commandPool = this->context->pool;
        grant.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        grant.commandBufferCount = 1;
        vkAllocateCommandBuffers(this->context->device, &grant, &this->context->buffer);

        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo desc_info{};
        desc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        desc_info.bindingCount = 1;
        desc_info.pBindings = &binding;
        vkCreateDescriptorSetLayout(this->context->device, &desc_info, nullptr, &this->context->descriptors);

        VkPushConstantRange range{};
        range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        range.offset = 0;
        range.size = 128;

        VkPipelineLayoutCreateInfo layout_info{};
        layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_info.setLayoutCount = 1;
        layout_info.pSetLayouts = &this->context->descriptors;
        layout_info.pushConstantRangeCount = 1;
        layout_info.pPushConstantRanges = &range;
        vkCreatePipelineLayout(this->context->device, &layout_info, nullptr, &this->context->layout);

        VkDescriptorPoolSize size{};
        size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        size.descriptorCount = 1024;

        VkDescriptorPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.maxSets = 1024;
        pool_info.poolSizeCount = 1;
        pool_info.pPoolSizes = &size;
        vkCreateDescriptorPool(this->context->device, &pool_info, nullptr, &this->context->assets);

        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = VK_FILTER_LINEAR;
        sampler_info.minFilter = VK_FILTER_LINEAR;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        vkCreateSampler(this->context->device, &sampler_info, nullptr, &this->context->sampler);
    }

    Vulkan::~Vulkan() {
        this->release();
    }

    void Vulkan::begin(const std::uint32_t width, const std::uint32_t height, const State& state) noexcept {
        if (!this->context || width == 0 || height == 0) return;
        if (!this->context->device) return;

        if (this->context->image && (this->context->width != width || this->context->height != height)) {
            this->sync();
            if (this->context->frame) vkDestroyFramebuffer(this->context->device, this->context->frame, nullptr);
            if (this->context->pass) vkDestroyRenderPass(this->context->device, this->context->pass, nullptr);
            if (this->context->view) vkDestroyImageView(this->context->device, this->context->view, nullptr);
            vkDestroyImage(this->context->device, this->context->image, nullptr);
            if (this->context->memory) vkFreeMemory(this->context->device, this->context->memory, nullptr);
            this->context->frame = VK_NULL_HANDLE;
            this->context->pass = VK_NULL_HANDLE;
            this->context->view = VK_NULL_HANDLE;
            this->context->image = VK_NULL_HANDLE;
            this->context->memory = VK_NULL_HANDLE;
        }

        if (!this->context->image) {
            this->context->width = width;
            this->context->height = height;

            VkImageCreateInfo img_info{};
            img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            img_info.imageType = VK_IMAGE_TYPE_2D;
            img_info.extent.width = width;
            img_info.extent.height = height;
            img_info.extent.depth = 1;
            img_info.mipLevels = 1;
            img_info.arrayLayers = 1;
            img_info.format = VK_FORMAT_R8G8B8A8_UNORM;
            img_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            img_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            img_info.samples = VK_SAMPLE_COUNT_1_BIT;
            img_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            vkCreateImage(this->context->device, &img_info, nullptr, &this->context->image);

            VkMemoryRequirements needs{};
            vkGetImageMemoryRequirements(this->context->device, this->context->image, &needs);

            VkMemoryAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = needs.size;

            VkPhysicalDeviceMemoryProperties mem_props{};
            vkGetPhysicalDeviceMemoryProperties(this->context->physical, &mem_props);
            std::uint32_t idx = 0;
            for (std::uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
                if ((needs.memoryTypeBits & (1 << i)) &&
                    (mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
                    idx = i;
                    break;
                }
            }
            alloc_info.memoryTypeIndex = idx;

            vkAllocateMemory(this->context->device, &alloc_info, nullptr, &this->context->memory);
            vkBindImageMemory(this->context->device, this->context->image, this->context->memory, 0);

            VkImageViewCreateInfo view_info{};
            view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.image = this->context->image;
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;

            vkCreateImageView(this->context->device, &view_info, nullptr, &this->context->view);

            VkAttachmentDescription attachment{};
            attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
            attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference reference{};
            reference.attachment = 0;
            reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &reference;

            VkRenderPassCreateInfo pass_info{};
            pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            pass_info.attachmentCount = 1;
            pass_info.pAttachments = &attachment;
            pass_info.subpassCount = 1;
            pass_info.pSubpasses = &subpass;

            vkCreateRenderPass(this->context->device, &pass_info, nullptr, &this->context->pass);

            VkFramebufferCreateInfo frame_info{};
            frame_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frame_info.renderPass = this->context->pass;
            frame_info.attachmentCount = 1;
            frame_info.pAttachments = &this->context->view;
            frame_info.width = width;
            frame_info.height = height;
            frame_info.layers = 1;

            vkCreateFramebuffer(this->context->device, &frame_info, nullptr, &this->context->frame);

            if (this->context->target) {
                *static_cast<VkImage*>(this->context->target) = this->context->image;
            }
        }

        VkCommandBufferBeginInfo start_info{};
        start_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(this->context->buffer, &start_info);

        VkRenderPassBeginInfo render_info{};
        render_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_info.renderPass = this->context->pass;
        render_info.framebuffer = this->context->frame;
        render_info.renderArea.offset = {0, 0};
        render_info.renderArea.extent = {width, height};

        VkClearValue clear = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        render_info.clearValueCount = 1;
        render_info.pClearValues = &clear;

        vkCmdBeginRenderPass(this->context->buffer, &render_info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
        VkRect2D scissor{{0, 0}, {width, height}};
        vkCmdSetViewport(this->context->buffer, 0, 1, &viewport);
        vkCmdSetScissor(this->context->buffer, 0, 1, &scissor);
    }

    void Vulkan::submit(const Buffer& stream) noexcept {
        if (!this->context) return;

        std::uint16_t pipeline = 0xFFFF;
        std::uint32_t texture = 0xFFFFFFFF;

        for (std::size_t i = 0; i < stream.count; ++i) {
            const Key key = stream.keys[i];
            const Command& command = stream.commands[i];

            if (key.bits.pipeline != pipeline) {
                pipeline = static_cast<std::uint16_t>(key.bits.pipeline);
                if (const auto match = this->context->pipelines.find(pipeline); match != this->context->pipelines.end()) {
                    vkCmdBindPipeline(this->context->buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, match->second);
                }
            }

            if (key.bits.texture != texture) {
                texture = static_cast<std::uint32_t>(key.bits.texture);
                if (const auto match = this->context->resources.find(texture); match != this->context->resources.end()) {
                    vkCmdBindDescriptorSets(this->context->buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->context->layout, 0, 1, &match->second.set, 0, nullptr);
                }
            }

            if (const auto match = this->context->buffers.find(static_cast<std::uint32_t>(key.bits.entity)); match != this->context->buffers.end()) {
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(this->context->buffer, 0, 1, &match->second.buffer, offsets);
            }

            vkCmdPushConstants(this->context->buffer, this->context->layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, 16, command.box);
            vkCmdDraw(this->context->buffer, 4, 1, 0, 0);
        }
    }

    void Vulkan::end() noexcept {
        if (!this->context) return;

        vkCmdEndRenderPass(this->context->buffer);
        vkEndCommandBuffer(this->context->buffer);

        VkSubmitInfo post{};
        post.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        post.commandBufferCount = 1;
        post.pCommandBuffers = &this->context->buffer;

        vkQueueSubmit(this->context->queue, 1, &post, VK_NULL_HANDLE);
        this->sync();
    }

    void Vulkan::dispose() noexcept {
        this->release();
    }

    void Vulkan::shader(const std::uint32_t id, const Shader& vertex, const Shader& pixel) noexcept {
        if (!this->context || !vertex.code || !pixel.code) return;

        this->sync();

        VkShaderModuleCreateInfo vs_info{};
        vs_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vs_info.codeSize = vertex.size;
        vs_info.pCode = static_cast<const std::uint32_t*>(vertex.code);
        VkShaderModule vs;
        vkCreateShaderModule(this->context->device, &vs_info, nullptr, &vs);

        VkShaderModuleCreateInfo ps_info{};
        ps_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        ps_info.codeSize = pixel.size;
        ps_info.pCode = static_cast<const std::uint32_t*>(pixel.code);
        VkShaderModule ps;
        vkCreateShaderModule(this->context->device, &ps_info, nullptr, &ps);

        VkPipelineShaderStageCreateInfo stages[2]{};
        stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = vs;
        stages[0].pName = "main";

        stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = ps;
        stages[1].pName = "main";

        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = sizeof(float) * 4;
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription attribute{};
        attribute.binding = 0;
        attribute.location = 0;
        attribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attribute.offset = 0;

        VkPipelineVertexInputStateCreateInfo input_state{};
        input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        input_state.vertexBindingDescriptionCount = 1;
        input_state.pVertexBindingDescriptions = &binding;
        input_state.vertexAttributeDescriptionCount = 1;
        input_state.pVertexAttributeDescriptions = &attribute;

        VkPipelineInputAssemblyStateCreateInfo assembly{};
        assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

        VkPipelineViewportStateCreateInfo view_state{};
        view_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        view_state.viewportCount = 1;
        view_state.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo raster{};
        raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        raster.polygonMode = VK_POLYGON_MODE_FILL;
        raster.cullMode = VK_CULL_MODE_NONE;
        raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        raster.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo sampling{};
        sampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        sampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState blend_state{};
        blend_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo blend{};
        blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blend.attachmentCount = 1;
        blend.pAttachments = &blend_state;

        VkDynamicState dynamic_items[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamic{};
        dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic.dynamicStateCount = 2;
        dynamic.pDynamicStates = dynamic_items;

        VkGraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = 2;
        pipeline_info.pStages = stages;
        pipeline_info.pVertexInputState = &input_state;
        pipeline_info.pInputAssemblyState = &assembly;
        pipeline_info.pViewportState = &view_state;
        pipeline_info.pRasterizationState = &raster;
        pipeline_info.pMultisampleState = &sampling;
        pipeline_info.pColorBlendState = &blend;
        pipeline_info.pDynamicState = &dynamic;
        pipeline_info.layout = this->context->layout;
        pipeline_info.renderPass = this->context->pass;

        VkPipeline pipeline;
        if (vkCreateGraphicsPipelines(this->context->device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) == VK_SUCCESS) {
            this->context->pipelines[static_cast<std::uint16_t>(id)] = pipeline;
        }

        vkDestroyShaderModule(this->context->device, vs, nullptr);
        vkDestroyShaderModule(this->context->device, ps, nullptr);
    }

    void Vulkan::mesh(const std::uint32_t id, const float* vertices, const std::size_t size) noexcept {
        if (!this->context || !vertices || size == 0) return;

        Allocation item{};
        item.size = size * sizeof(float);

        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = item.size;
        buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vkCreateBuffer(this->context->device, &buffer_info, nullptr, &item.buffer);

        VkMemoryRequirements needs{};
        vkGetBufferMemoryRequirements(this->context->device, item.buffer, &needs);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = needs.size;

        VkPhysicalDeviceMemoryProperties mem_props{};
        vkGetPhysicalDeviceMemoryProperties(this->context->physical, &mem_props);
        std::uint32_t idx = 0;
        for (std::uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if ((needs.memoryTypeBits & (1 << i)) &&
                (mem_props.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
                idx = i;
                break;
            }
        }
        alloc_info.memoryTypeIndex = idx;

        vkAllocateMemory(this->context->device, &alloc_info, nullptr, &item.memory);
        vkBindBufferMemory(this->context->device, item.buffer, item.memory, 0);

        void* ptr = nullptr;
        vkMapMemory(this->context->device, item.memory, 0, item.size, 0, &ptr);
        std::memcpy(ptr, vertices, item.size);
        vkUnmapMemory(this->context->device, item.memory);

        this->context->buffers[id] = item;
    }

    void Vulkan::update(const std::uint32_t id, const float* vertices, const std::size_t size) noexcept {
        if (!this->context || !vertices || size == 0) return;

        if (const auto match = this->context->buffers.find(id); match != this->context->buffers.end()) {
            void* ptr = nullptr;
            vkMapMemory(this->context->device, match->second.memory, 0, size * sizeof(float), 0, &ptr);
            std::memcpy(ptr, vertices, size * sizeof(float));
            vkUnmapMemory(this->context->device, match->second.memory);
        }
    }

    void Vulkan::free(const std::uint32_t id) noexcept {
        if (!this->context) return;
        this->sync();

        if (const auto match = this->context->buffers.find(id); match != this->context->buffers.end()) {
            vkDestroyBuffer(this->context->device, match->second.buffer, nullptr);
            vkFreeMemory(this->context->device, match->second.memory, nullptr);
            this->context->buffers.erase(id);
        }
    }

    void Vulkan::texture(const std::uint32_t id, const std::uint8_t* pixels, const std::uint32_t width, const std::uint32_t height) noexcept {
        if (!this->context || !pixels || width == 0 || height == 0) return;

        this->sync();

        Entry item{};
        VkDeviceSize bytes = width * height * 4;

        VkBuffer staging;
        VkDeviceMemory staging_mem;

        VkBufferCreateInfo buf_info{};
        buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buf_info.size = bytes;
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateBuffer(this->context->device, &buf_info, nullptr, &staging);

        VkMemoryRequirements buf_needs{};
        vkGetBufferMemoryRequirements(this->context->device, staging, &buf_needs);

        VkPhysicalDeviceMemoryProperties mem_props{};
        vkGetPhysicalDeviceMemoryProperties(this->context->physical, &mem_props);
        std::uint32_t host_idx = 0;
        for (std::uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if ((buf_needs.memoryTypeBits & (1 << i)) &&
                (mem_props.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) {
                host_idx = i;
                break;
            }
        }

        VkMemoryAllocateInfo buf_alloc{};
        buf_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        buf_alloc.allocationSize = buf_needs.size;
        buf_alloc.memoryTypeIndex = host_idx;
        vkAllocateMemory(this->context->device, &buf_alloc, nullptr, &staging_mem);
        vkBindBufferMemory(this->context->device, staging, staging_mem, 0);

        void* ptr = nullptr;
        vkMapMemory(this->context->device, staging_mem, 0, bytes, 0, &ptr);
        std::memcpy(ptr, pixels, bytes);
        vkUnmapMemory(this->context->device, staging_mem);

        VkImageCreateInfo img_info{};
        img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        img_info.imageType = VK_IMAGE_TYPE_2D;
        img_info.extent.width = width;
        img_info.extent.height = height;
        img_info.extent.depth = 1;
        img_info.mipLevels = 1;
        img_info.arrayLayers = 1;
        img_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        img_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        img_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        img_info.samples = VK_SAMPLE_COUNT_1_BIT;
        img_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateImage(this->context->device, &img_info, nullptr, &item.image);

        VkMemoryRequirements img_needs{};
        vkGetImageMemoryRequirements(this->context->device, item.image, &img_needs);

        std::uint32_t dev_idx = 0;
        for (std::uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if ((img_needs.memoryTypeBits & (1 << i)) &&
                (mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
                dev_idx = i;
                break;
            }
        }

        VkMemoryAllocateInfo img_alloc{};
        img_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        img_alloc.allocationSize = img_needs.size;
        img_alloc.memoryTypeIndex = dev_idx;
        vkAllocateMemory(this->context->device, &img_alloc, nullptr, &item.memory);
        vkBindImageMemory(this->context->device, item.image, item.memory, 0);

        VkCommandBufferBeginInfo cmd_begin{};
        cmd_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(this->context->buffer, &cmd_begin);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = item.image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(this->context->buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkBufferImageCopy region{};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(this->context->buffer, staging, item.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // Fixed pipeline stage flag name here
        vkCmdPipelineBarrier(this->context->buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        vkEndCommandBuffer(this->context->buffer);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &this->context->buffer;
        vkQueueSubmit(this->context->queue, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(this->context->queue);

        vkDestroyBuffer(this->context->device, staging, nullptr);
        vkFreeMemory(this->context->device, staging_mem, nullptr);

        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = item.image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;
        vkCreateImageView(this->context->device, &view_info, nullptr, &item.view);

        VkDescriptorSetAllocateInfo set_alloc{};
        set_alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        set_alloc.descriptorPool = this->context->assets;
        set_alloc.descriptorSetCount = 1;
        set_alloc.pSetLayouts = &this->context->descriptors;
        vkAllocateDescriptorSets(this->context->device, &set_alloc, &item.set);

        VkDescriptorImageInfo img_desc{};
        img_desc.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        img_desc.imageView = item.view;
        img_desc.sampler = this->context->sampler;

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = item.set;
        write.dstBinding = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = &img_desc;
        vkUpdateDescriptorSets(this->context->device, 1, &write, 0, nullptr);

        this->context->resources[id] = item;
    }

    void Vulkan::unload(const std::uint32_t id) noexcept {
        if (!this->context) return;
        this->sync();

        if (const auto match = this->context->resources.find(id); match != this->context->resources.end()) {
            vkDestroyImageView(this->context->device, match->second.view, nullptr);
            vkDestroyImage(this->context->device, match->second.image, nullptr);
            vkFreeMemory(this->context->device, match->second.memory, nullptr);
            this->context->resources.erase(id);
        }
    }

    void Vulkan::surface(const std::uint32_t id, const std::uint32_t width, const std::uint32_t height) noexcept {
        if (!this->context) return;

        Surface item{};

        VkImageCreateInfo img_info{};
        img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        img_info.imageType = VK_IMAGE_TYPE_2D;
        img_info.extent.width = width;
        img_info.extent.height = height;
        img_info.extent.depth = 1;
        img_info.mipLevels = 1;
        img_info.arrayLayers = 1;
        img_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        img_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        img_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        img_info.samples = VK_SAMPLE_COUNT_1_BIT;
        img_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateImage(this->context->device, &img_info, nullptr, &item.image);

        VkMemoryRequirements needs{};
        vkGetImageMemoryRequirements(this->context->device, item.image, &needs);

        VkPhysicalDeviceMemoryProperties mem_props{};
        vkGetPhysicalDeviceMemoryProperties(this->context->physical, &mem_props);
        std::uint32_t idx = 0;
        for (std::uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if ((needs.memoryTypeBits & (1 << i)) &&
                (mem_props.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
                idx = i;
                break;
            }
        }

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = needs.size;
        alloc_info.memoryTypeIndex = idx;
        vkAllocateMemory(this->context->device, &alloc_info, nullptr, &item.memory);
        vkBindImageMemory(this->context->device, item.image, item.memory, 0);

        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = item.image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;
        vkCreateImageView(this->context->device, &view_info, nullptr, &item.view);

        VkFramebufferCreateInfo frame_info{};
        frame_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frame_info.renderPass = this->context->pass;
        frame_info.attachmentCount = 1;
        frame_info.pAttachments = &item.view;
        frame_info.width = width;
        frame_info.height = height;
        frame_info.layers = 1;
        vkCreateFramebuffer(this->context->device, &frame_info, nullptr, &item.framebuffer);

        this->context->surfaces[id] = item;
    }

    void Vulkan::remove(const std::uint32_t id) noexcept {
        if (!this->context) return;
        this->sync();

        if (const auto match = this->context->surfaces.find(id); match != this->context->surfaces.end()) {
            vkDestroyFramebuffer(this->context->device, match->second.framebuffer, nullptr);
            vkDestroyImageView(this->context->device, match->second.view, nullptr);
            vkDestroyImage(this->context->device, match->second.image, nullptr);
            vkFreeMemory(this->context->device, match->second.memory, nullptr);
            this->context->surfaces.erase(id);
        }
    }

    void Vulkan::release() noexcept {
        if (!this->context) return;

        if (this->context->device) {
            this->sync();

            // Using std::views::values for pipeline tracking cleanup
            for (const auto& pipeline : this->context->pipelines | std::views::values) {
                vkDestroyPipeline(this->context->device, pipeline, nullptr);
            }
            this->context->pipelines.clear();

            // Using std::views::values for buffer cleanup
            for (const auto& allocation : this->context->buffers | std::views::values) {
                vkDestroyBuffer(this->context->device, allocation.buffer, nullptr);
                vkFreeMemory(this->context->device, allocation.memory, nullptr);
            }
            this->context->buffers.clear();

            // Using std::views::values for resource cleanup
            for (const auto& resource : this->context->resources | std::views::values) {
                vkDestroyImageView(this->context->device, resource.view, nullptr);
                vkDestroyImage(this->context->device, resource.image, nullptr);
                vkFreeMemory(this->context->device, resource.memory, nullptr);
            }
            this->context->resources.clear();

            // Using std::views::values for surface framebuffer cleanup
            for (const auto& surface : this->context->surfaces | std::views::values) {
                vkDestroyFramebuffer(this->context->device, surface.framebuffer, nullptr);
                vkDestroyImageView(this->context->device, surface.view, nullptr);
                vkDestroyImage(this->context->device, surface.image, nullptr);
                vkFreeMemory(this->context->device, surface.memory, nullptr);
            }
            this->context->surfaces.clear();

            if (this->context->sampler) vkDestroySampler(this->context->device, this->context->sampler, nullptr);
            if (this->context->assets) vkDestroyDescriptorPool(this->context->device, this->context->assets, nullptr);
            if (this->context->layout) vkDestroyPipelineLayout(this->context->device, this->context->layout, nullptr);
            if (this->context->descriptors) vkDestroyDescriptorSetLayout(this->context->device, this->context->descriptors, nullptr);
            if (this->context->frame) vkDestroyFramebuffer(this->context->device, this->context->frame, nullptr);
            if (this->context->pass) vkDestroyRenderPass(this->context->device, this->context->pass, nullptr);
            if (this->context->view) vkDestroyImageView(this->context->device, this->context->view, nullptr);
            if (this->context->image) vkDestroyImage(this->context->device, this->context->image, nullptr);
            if (this->context->memory) vkFreeMemory(this->context->device, this->context->memory, nullptr);
            if (this->context->pool) vkDestroyCommandPool(this->context->device, this->context->pool, nullptr);
            vkDestroyDevice(this->context->device, nullptr);
        }

        if (this->context->instance) {
            vkDestroyInstance(this->context->instance, nullptr);
        }

        delete this->context;
        this->context = nullptr;
    }

    void Vulkan::sync() const noexcept {
        if (!this->context) return;
        if (this->context->queue) {
            vkQueueWaitIdle(this->context->queue);
        }
    }

}