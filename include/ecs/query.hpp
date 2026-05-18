#pragma once
#include <vector>
#include <memory>
#include "archetype.hpp"

class Query {
    const std::vector<std::unique_ptr<Archetype>>& archetypes;
    Mask require;
    Mask reject;

public:
    explicit Query(const std::vector<std::unique_ptr<Archetype>>& archetypes);

    Query& with(uint32_t target);
    Query& without(uint32_t target);

    // Template implementations must stay in the header file
    template<typename Callback>
    void run(Callback callback, const std::vector<uint32_t>& targets) {
        for (const auto& archetype : archetypes) {
            if ((archetype->mask & require) != require) continue;
            if ((archetype->mask & reject).any()) continue;

            std::shared_lock lock(archetype->mutex); // Safe reading

            std::vector<void*> arrays;
            for (uint32_t target : targets) {
                const size_t column = archetype->mapping[target];
                arrays.push_back(archetype->storage[column].data());
            }

            // Passes the entity count, entity array, and raw component arrays directly to you
            callback(archetype->entities.size(), archetype->entities.data(), arrays);
        }
    }
};