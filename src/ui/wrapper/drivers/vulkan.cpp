#include "ui/wrapper/drivers/vulkan.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <cstring>

namespace core::ui::drivers {

namespace {
    struct Context {
        VkInstance instance{VK_NULL_HANDLE};
        VkPhysicalDevice physical{VK_NULL_HANDLE};
        VkDevice logical{VK_NULL_HANDLE};
        VkQueue queue{VK_NULL_HANDLE};
        VkCommandPool pool{VK_NULL_HANDLE};
        VkCommandBuffer buffer{VK_NULL_HANDLE};
        VkFence fence{VK_NULL_HANDLE};

        VkBuffer vertices{VK_NULL_HANDLE};
        VkDeviceMemory vertex{VK_NULL_HANDLE};
        VkBuffer indices{VK_NULL_HANDLE};
        VkDeviceMemory index{VK_NULL_HANDLE};
    };

    uint32_t find(VkPhysicalDevice physical, uint32_t filter) {
        VkPhysicalDeviceMemoryProperties properties;
        vkGetPhysicalDeviceMemoryProperties(physical, &properties);
        for (uint32_t slot = 0; slot < properties.memoryTypeCount; slot++) {
            if ((filter & (1 << slot)) && (properties.memoryTypes[slot].propertyFlags & 6) == 6) {
                return slot;
            }
        }
        return 0xFFFFFFFF;
    }

    void allocate(VkDevice logical, VkPhysicalDevice physical, VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VkDeviceMemory& memory) {
        VkBufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.size = size;
        info.usage = usage;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(logical, &info, nullptr, &buffer) != VK_SUCCESS) return;

        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(logical, buffer, &requirements);

        VkMemoryAllocateInfo allocation{};
        allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocation.allocationSize = requirements.size;
        allocation.memoryTypeIndex = find(physical, requirements.memoryTypeBits);

        if (vkAllocateMemory(logical, &allocation, nullptr, &memory) != VK_SUCCESS) return;
        vkBindBufferMemory(logical, buffer, memory, 0);
    }
}

Vulkan::Vulkan(const void* target) noexcept {
    (void)target;
    auto state = std::make_unique<Context>();

    VkApplicationInfo app{};
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo setup{};
    setup.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    setup.pApplicationInfo = &app;

    if (vkCreateInstance(&setup, nullptr, &state->instance) != VK_SUCCESS) return;

    uint32_t count = 0;
    if (vkEnumeratePhysicalDevices(state->instance, &count, nullptr) != VK_SUCCESS || count == 0) {
        dispose(); return;
    }
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(state->instance, &count, devices.data());
    state->physical = devices[0];

    uint32_t amount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(state->physical, &amount, nullptr);
    std::vector<VkQueueFamilyProperties> families(amount);
    vkGetPhysicalDeviceQueueFamilyProperties(state->physical, &amount, families.data());

    uint32_t select = 0xFFFFFFFF;
    for (uint32_t slot = 0; slot < amount; slot++) {
        if (families[slot].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            select = slot;
            break;
        }
    }
    if (select == 0xFFFFFFFF) { dispose(); return; }

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue{};
    queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue.queueFamilyIndex = select;
    queue.queueCount = 1;
    queue.pQueuePriorities = &priority;

    VkDeviceCreateInfo device{};
    device.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device.queueCreateInfoCount = 1;
    device.pQueueCreateInfos = &queue;

    if (vkCreateDevice(state->physical, &device, nullptr, &state->logical) != VK_SUCCESS) {
        dispose(); return;
    }
    vkGetDeviceQueue(state->logical, select, 0, &state->queue);

    VkCommandPoolCreateInfo pool{};
    pool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool.queueFamilyIndex = select;

    if (vkCreateCommandPool(state->logical, &pool, nullptr, &state->pool) != VK_SUCCESS) {
        dispose(); return;
    }

    VkCommandBufferAllocateInfo allocation{};
    allocation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocation.commandPool = state->pool;
    allocation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocation.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(state->logical, &allocation, &state->buffer) != VK_SUCCESS) {
        dispose(); return;
    }

    VkFenceCreateInfo fence{};
    fence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(state->logical, &fence, nullptr, &state->fence) != VK_SUCCESS) {
        dispose(); return;
    }

    handle = state.release();
}

void Vulkan::begin(const std::uint32_t width, const std::uint32_t height) const noexcept {
    auto* state = static_cast<Context*>(handle);
    if (!state) return;
    (void)width; (void)height;

    vkWaitForFences(state->logical, 1, &state->fence, VK_TRUE, UINT64_MAX);
    vkResetFences(state->logical, 1, &state->fence);

    vkResetCommandBuffer(state->buffer, 0);

    VkCommandBufferBeginInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(state->buffer, &info);
}

void Vulkan::draw(const void* source, const std::size_t count, const std::uint32_t* keys, std::size_t items) const noexcept {
    auto* state = static_cast<Context*>(handle);
    if (!state || !source || !keys) return;

    if (state->vertices) {
        vkDestroyBuffer(state->logical, state->vertices, nullptr);
        vkFreeMemory(state->logical, state->vertex, nullptr);
    }
    if (state->indices) {
        vkDestroyBuffer(state->logical, state->indices, nullptr);
        vkFreeMemory(state->logical, state->index, nullptr);
    }

    const VkDeviceSize bytes = count * sizeof(float) * 3;
    allocate(state->logical, state->physical, bytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, state->vertices, state->vertex);

    void* address;
    vkMapMemory(state->logical, state->vertex, 0, bytes, 0, &address);
    std::memcpy(address, source, bytes);
    vkUnmapMemory(state->logical, state->vertex);

    const VkDeviceSize range = items * sizeof(std::uint32_t);
    allocate(state->logical, state->physical, range, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, state->indices, state->index);

    void* location;
    vkMapMemory(state->logical, state->index, 0, range, 0, &location);
    std::memcpy(location, keys, range);
    vkUnmapMemory(state->logical, state->index);

    const VkBuffer bindings[] = { state->vertices };
    constexpr VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(state->buffer, 0, 1, bindings, offsets);
    vkCmdBindIndexBuffer(state->buffer, state->indices, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(state->buffer, static_cast<std::uint32_t>(items), 1, 0, 0, 0);
}

void Vulkan::end() const noexcept {
    const auto* state = static_cast<Context*>(handle);
    if (!state) return;

    vkEndCommandBuffer(state->buffer);

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &state->buffer;

    vkQueueSubmit(state->queue, 1, &submit, state->fence);
}

void Vulkan::dispose() noexcept {
    if (handle) {
        auto* state = static_cast<Context*>(handle);
        if (state->logical) {
            vkDeviceWaitIdle(state->logical);

            if (state->vertices) {
                vkDestroyBuffer(state->logical, state->vertices, nullptr);
                vkFreeMemory(state->logical, state->vertex, nullptr);
            }
            if (state->indices) {
                vkDestroyBuffer(state->logical, state->indices, nullptr);
                vkFreeMemory(state->logical, state->index, nullptr);
            }

            if (state->fence) vkDestroyFence(state->logical, state->fence, nullptr);
            if (state->pool) vkDestroyCommandPool(state->logical, state->pool, nullptr);
            vkDestroyDevice(state->logical, nullptr);
        }
        if (state->instance) {
            vkDestroyInstance(state->instance, nullptr);
        }
        delete state;
        handle = nullptr;
    }
}

}