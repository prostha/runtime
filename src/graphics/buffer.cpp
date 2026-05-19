#include "../include/graphics/buffer.hpp"
#include "../include/graphics/context.hpp"
#include <stdexcept>
#include <cstring>

namespace voxyl::graphics {

    Buffer::Buffer(const Context& context, VkDeviceSize bytes, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags)
        : core(context.device()), silicon(context.hardware()), bytes(bytes), region(nullptr) {
        VkBufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.size = bytes;
        info.usage = usage;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(core, &info, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("error");
        }

        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(core, buffer, &requirements);

        VkMemoryAllocateInfo allocate{};
        allocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate.allocationSize = requirements.size;
        allocate.memoryTypeIndex = find(requirements.memoryTypeBits, flags);

        if (vkAllocateMemory(core, &allocate, nullptr, &memory) != VK_SUCCESS) {
            throw std::runtime_error("error");
        }

        vkBindBufferMemory(core, buffer, memory, 0);
    }

    Buffer::~Buffer() {
        unmap();
        if (buffer != VK_NULL_HANDLE) vkDestroyBuffer(core, buffer, nullptr);
        if (memory != VK_NULL_HANDLE) vkFreeMemory(core, memory, nullptr);
    }

    Buffer::Buffer(Buffer&& other) noexcept
        : core(other.core), silicon(other.silicon), buffer(other.buffer), memory(other.memory), bytes(other.bytes), region(other.region) {
        other.buffer = VK_NULL_HANDLE;
        other.memory = VK_NULL_HANDLE;
        other.region = nullptr;
        other.bytes = 0;
    }

    Buffer& Buffer::operator=(Buffer&& other) noexcept {
        if (this != &other) {
            unmap();
            if (buffer != VK_NULL_HANDLE) vkDestroyBuffer(core, buffer, nullptr);
            if (memory != VK_NULL_HANDLE) vkFreeMemory(core, memory, nullptr);
            core = other.core;
            silicon = other.silicon;
            buffer = other.buffer;
            memory = other.memory;
            bytes = other.bytes;
            region = other.region;
            other.buffer = VK_NULL_HANDLE;
            other.memory = VK_NULL_HANDLE;
            other.region = nullptr;
            other.bytes = 0;
        }
        return *this;
    }

    void Buffer::map(VkDeviceSize shift, VkDeviceSize span) {
        if (!region) {
            if (vkMapMemory(core, memory, shift, span, 0, &region) != VK_SUCCESS) {
                throw std::runtime_error("error");
            }
        }
    }

    void Buffer::unmap() {
        if (region) {
            vkUnmapMemory(core, memory);
            region = nullptr;
        }
    }

    void Buffer::write(const void* data, VkDeviceSize span, VkDeviceSize shift) {
        bool temp = !region;
        if (temp) map(shift, span);
        std::memcpy(static_cast<char*>(region) + (temp ? 0 : shift), data, static_cast<std::size_t>(span));
        if (temp) unmap();
    }

    uint32_t Buffer::find(uint32_t filter, VkMemoryPropertyFlags flags) const {
        VkPhysicalDeviceMemoryProperties properties;
        vkGetPhysicalDeviceMemoryProperties(silicon, &properties);
        for (uint32_t index = 0; index < properties.memoryTypeCount; ++index) {
            if ((filter & (1 << index)) && (properties.memoryTypes[index].propertyFlags & flags) == flags) {
                return index;
            }
        }
        throw std::runtime_error("error");
    }

}