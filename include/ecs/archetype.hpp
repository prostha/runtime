#pragma once
#include <vector>
#include <shared_mutex>
#include <atomic>
#include <unordered_map>
#include "mask.hpp"
#include "types.hpp"

struct Archetype {
    Mask mask;
    std::vector<Entity> entities;
    std::vector<std::vector<uint8_t>> storage;
    std::vector<size_t> sizes;
    std::unordered_map<uint32_t, size_t> mapping;
    std::atomic<uint32_t> version{0};
    mutable std::shared_mutex mutex;

    void add(Entity entity);
    void remove(size_t index);
};