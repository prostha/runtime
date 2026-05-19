#pragma once
#include <memory>
#include <vector>
#include "archetype.hpp"
#include "query.hpp"

namespace voxyl::ecs {

    class World {
    public:
        World() = default;
        ~World() = default;

        World(const World&) = delete;
        World& operator=(const World&) = delete;
        World(World&&) noexcept = default;
        World& operator=(World&&) noexcept = default;

        std::uint32_t enroll(std::size_t size);
        [[nodiscard]] Entity spawn();
        void kill(Entity entity);

        void* attach(Entity entity, std::uint32_t target, const void* data = nullptr);
        [[nodiscard]] void* get(Entity entity, std::uint32_t target);
        [[nodiscard]] bool has(Entity entity, std::uint32_t target) const;
        void detach(Entity entity, std::uint32_t target);

        [[nodiscard]] Query query() const;

    private:
        [[nodiscard]] Archetype* locate(const Mask& mask);
        void migrate(Entity entity, const Mask& mask);

        std::vector<std::unique_ptr<Archetype>> archetypes;
        std::vector<Record> records;
        std::vector<Entity> pool;
        std::vector<std::size_t> sizes;
        Entity cursor = 0;
    };

}