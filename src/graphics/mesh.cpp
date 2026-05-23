#include "../include/graphics/mesh.hpp"
#include <memory>

namespace voxyl::graphics {

    Mesh::Mesh(const Context& context, const std::vector<Vertex>& points)
        : total(static_cast<uint32_t>(points.size())) {
        VkDeviceSize allocation = points.size() * sizeof(Vertex);
        vertices = std::make_unique<Buffer>(context, allocation, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vertices->write(points.data(), allocation);
    }

}