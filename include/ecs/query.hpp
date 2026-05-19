#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <future>
#include "archetype.hpp"

namespace voxyl {
    struct Gpu;
}

namespace voxyl::ecs {

    class Query {
    public:
        using Callback = std::function<void(std::size_t count, const Entity* entities, const std::vector<void*>& components)>;
        
        explicit Query(const std::vector<std::unique_ptr<Archetype>>& archetypes, Gpu* device = nullptr) noexcept;

        Query& with(std::uint32_t target);
        Query& without(std::uint32_t target);
        Query& any(const std::vector<std::uint32_t>& targets);

        [[nodiscard]] bool has(std::uint32_t target) const noexcept;

        void run(const Callback& callback, const std::vector<std::uint32_t>& targets) const;

    private:
        void cpu(const Callback& callback, const std::vector<std::uint32_t>& targets) const;
        [[nodiscard]] bool gpu(const std::vector<std::uint32_t>& targets) const;

        const std::vector<std::unique_ptr<Archetype>>& m_archetypes;
        Gpu* m_device;
        Mask m_require;
        Mask m_reject;
        Mask m_allow;
    };

}