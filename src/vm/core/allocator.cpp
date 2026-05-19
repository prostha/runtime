#include "../include/vm/core/allocator.hpp"
#include <cstdlib>
#include <cstdio>
#include <cassert>

namespace allocator {

    Allocator::Allocator(const size_t capacity)
        : capacity(capacity) {}

    void *Allocator::manage(void *context, void *pointer, size_t previous, size_t next) {
        assert(context != nullptr);

        auto *instance = static_cast<Allocator*>(context);

        if (pointer != nullptr && previous > 0) {
            const size_t decrement = previous <= instance->consumed ? previous : instance->consumed;
            instance->consumed -= decrement;
        }

        if (next == 0) {
            std::free(pointer);
            return nullptr;
        }

        if (next > instance->capacity - instance->consumed) {
            std::fprintf(stderr, "allocator: out of memory next=%zu consumed=%zu capacity=%zu\n", next, instance->consumed, instance->capacity);
            return nullptr;
        }

        void *block = std::realloc(pointer, next);
        if (block == nullptr) {
            std::fprintf(stderr, "allocator: realloc failed next=%zu\n", next);
            return nullptr;
        }

        instance->consumed += next;
        return block;
    }

}