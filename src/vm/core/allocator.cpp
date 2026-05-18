#include "../include/vm/core/allocator.hpp"
#include <cstdlib>
#include <cstdio>
#include <cassert>

namespace polyblox {

    Allocator::Allocator(size_t capacity)
        : capacity(capacity) {}

    void *Allocator::allocator_func(void *context, void *pointer, size_t previous, size_t next) {
        assert(context != nullptr);

        auto *allocator = static_cast<Allocator*>(context);

        if (pointer != nullptr && previous > 0) {
            const size_t sub = previous <= allocator->consumed ? previous : allocator->consumed;
            allocator->consumed -= sub;
        }

        if (next == 0) {
            std::free(pointer);
            return nullptr;
        }

        if (next > allocator->capacity - allocator->consumed) {
            std::fprintf(stderr, "allocator: out of memory next=%zu consumed=%zu capacity=%zu\n", next, allocator->consumed, allocator->capacity);
            return nullptr;
        }

        void *block = std::realloc(pointer, next);
        if (block == nullptr) {
            std::fprintf(stderr, "allocator: realloc failed next=%zu\n", next);
            return nullptr;
        }

        allocator->consumed += next;
        return block;
    }

}