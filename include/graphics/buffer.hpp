#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

namespace voxyl::graphics {

    class Context;

    class Buffer {
    public:
        Buffer(const Context& context, VkDeviceSize bytes, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags);
        ~Buffer();

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&& other) noexcept;
        Buffer& operator=(Buffer&& other) noexcept;

        void map(VkDeviceSize shift = 0, VkDeviceSize span = VK_WHOLE_SIZE);
        void unmap();
        void write(const void* data, VkDeviceSize span, VkDeviceSize shift = 0);

        [[nodiscard]] VkBuffer handle() const { return buffer; }
        [[nodiscard]] VkDeviceSize span() const { return bytes; }

    private:
        [[nodiscard]] uint32_t find(uint32_t filter, VkMemoryPropertyFlags flags) const;

        VkDevice core;
        VkPhysicalDevice silicon;

        VkBuffer buffer;
        VkDeviceMemory memory;
        VkDeviceSize bytes;

        void* region;
    };

}