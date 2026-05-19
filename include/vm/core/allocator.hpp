#pragma once

#include <cstddef>

namespace allocator {

    class Allocator {
    public:
        explicit Allocator(size_t capacity);
        ~Allocator() = default;

        static void* manage(void* context, void* pointer, size_t previous, size_t next);

        size_t consumed = 0;
        size_t capacity = 0;
    };

}