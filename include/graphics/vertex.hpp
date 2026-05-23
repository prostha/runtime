#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace voxyl::graphics {

    struct Vertex {
        float x;
        float y;
        float z;
        float r;
        float g;
        float b;
        float u;
        float v;

        static VkVertexInputBindingDescription binding();
        static std::vector<VkVertexInputAttributeDescription> attributes();
    };

}