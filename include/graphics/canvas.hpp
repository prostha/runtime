#pragma once

#include "vertex.hpp"
#include <vector>

namespace voxyl::graphics {

    class Canvas {
    public:
        void clear();
        void line(float x1, float y1, float x2, float y2, float r, float g, float b);
        void rectangle(float x, float y, float w, float h, float r, float g, float b);
        void circle(float x, float y, float radius, float r, float g, float b);

        [[nodiscard]] const std::vector<Vertex>& data() const { return vertices; }

    private:
        std::vector<Vertex> vertices;
    };

}