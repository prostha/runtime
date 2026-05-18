#include "../include/ecs/query.hpp"

Query::Query(const std::vector<std::unique_ptr<Archetype>>& archetypes)
    : archetypes(archetypes) {}

Query& Query::with(const uint32_t target) {
    require.set(target);
    return *this;
}

Query& Query::without(const uint32_t target) {
    reject.set(target);
    return *this;
}