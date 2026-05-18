#pragma once

#include <cstddef>

namespace polyblox {

    class Allocator {
    public:
        explicit Allocator(size_t capacity);
        ~Allocator() = default;

        static void* allocator_func(void* context, void* pointer, size_t previous, size_t next);

        size_t consumed = 0;
        size_t capacity = 0;
    };

}

#endif