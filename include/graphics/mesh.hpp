#pragma once

#include "buffer.hpp"
#include "vertex.hpp"
#include <vector>
#include <memory>

namespace voxyl::graphics {

    class Mesh {
    public:
        Mesh(const Context& context, const std::vector<Vertex>& points);

        [[nodiscard]] const Buffer& source() const { return *vertices; }
        [[nodiscard]] uint32_t count() const { return total; }

    private:
        std::unique_ptr<Buffer> vertices;
        uint32_t total;
    };

}