#include "../include/ecs/world.hpp"
#include <cstring>

uint32_t World::enroll(const size_t size) {
    sizes.push_back(size);
    return sizes.size() - 1;
}

Entity World::spawn() {
    Entity entity;
    if (!pool.empty()) {
        entity = pool.back();
        pool.pop_back();
    } else {
        entity = cursor++;
    }
    if (entity >= records.size()) records.resize(entity + 1);

    constexpr Mask mask;
    migrate(entity, mask);
    return entity;
}

void World::kill(const Entity entity) {
    auto [archetype, index] = records[entity];
    if (archetype) {
        archetype->remove(index);
        // Fix the index of the entity that got swapped to fill the hole
        if (index < archetype->entities.size()) {
            const Entity swapped = archetype->entities[index];
            records[swapped].index = index;
        }
    }
    records[entity] = {nullptr, 0};
    pool.push_back(entity);
}

void* World::attach(const Entity entity, const uint32_t target, const void* data) {
    Mask mask = records[entity].archetype ? records[entity].archetype->mask : Mask();
    mask.set(target);
    migrate(entity, mask);

    auto [archetype, index] = records[entity];
    const size_t column = archetype->mapping[target];
    void* memory = &archetype->storage[column][index * sizes[target]];

    if (data) std::memcpy(memory, data, sizes[target]);
    return memory;
}

[[nodiscard]] Query World::query() const {
    return Query(archetypes);
}

Archetype* World::locate(const Mask &mask) {
    for (const auto& archetype : archetypes) {
        if (archetype->mask == mask) return archetype.get();
    }

    auto archetype = std::make_unique<Archetype>();
    archetype->mask = mask;
    for (size_t index = 0; index < sizes.size(); ++index) {
        if (mask.test(index)) {
            archetype->mapping[index] = archetype->storage.size();
            archetype->storage.emplace_back();
            archetype->sizes.push_back(sizes[index]);
        }
    }
    archetypes.push_back(std::move(archetype));
    return archetypes.back().get();
}

void World::migrate(const Entity entity, const Mask &mask) {
    auto [archetype, index] = records[entity];
    Archetype* destination = locate(mask);

    if (archetype == destination) return;

    destination->add(entity);
    const size_t slot = destination->entities.size() - 1;

    if (archetype) {
        std::shared_lock lock(archetype->mutex); // Ensure no one writes while we read
        for (auto const& [target, column] : archetype->mapping) {
            if (destination->mapping.contains(target)) {
                const size_t lane = destination->mapping[target];
                std::memcpy(
                    &destination->storage[lane][slot * sizes[target]],
                    &archetype->storage[column][index * sizes[target]],
                    sizes[target]
                );
            }
        }
        archetype->remove(index);
        if (index < archetype->entities.size()) {
            const Entity swapped = archetype->entities[index];
            records[swapped].index = index;
        }
    }
    records[entity] = {destination, slot};
}