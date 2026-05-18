#include "../include/ecs/archetype.hpp"
#include <cstring>

void Archetype::add(const Entity entity) {
    std::unique_lock lock(mutex);
    entities.push_back(entity);
    for (size_t index = 0; index < storage.size(); ++index) {
        storage[index].resize(storage[index].size() + sizes[index], 0);
    }
    version.fetch_add(1, std::memory_order_release);
}

void Archetype::remove(const size_t index) {
    std::unique_lock lock(mutex);
    if (const size_t last = entities.size() - 1; index < last) {
        entities[index] = entities[last];
        for (size_t column = 0; column < storage.size(); ++column) {
            const size_t size = sizes[column];
            std::memcpy(&storage[column][index * size], &storage[column][last * size], size);
        }
    }
    entities.pop_back();
    for (size_t column = 0; column < storage.size(); ++column) {
        storage[column].resize(storage[column].size() - sizes[column]);
    }
    version.fetch_add(1, std::memory_order_release);
}