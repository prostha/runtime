#pragma once
#include <memory>
#include <vector>
#include "archetype.hpp"
#include "query.hpp"

class World {
    std::vector<std::unique_ptr<Archetype>> archetypes;
    std::vector<Record> records;
    std::vector<Entity> pool;
    std::vector<size_t> sizes;
    Entity cursor = 0;

public:
    uint32_t enroll(size_t size);
    Entity spawn();
    void kill(Entity entity);
    void* attach(Entity entity, uint32_t target, const void* data = nullptr);

    [[nodiscard]] Query query() const;

private:
    Archetype* locate(const Mask &mask);
    void migrate(Entity entity, const Mask &mask);
};