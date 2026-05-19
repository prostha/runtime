#include "../../include/ecs/query.hpp"
#include <shared_mutex>
#include <future>

namespace voxyl::ecs {

    Query::Query(const std::vector<std::unique_ptr<Archetype>>& archetypes, Gpu* device) noexcept
        : m_archetypes(archetypes), m_device(device) {}

    Query& Query::with(const std::uint32_t target) {
        m_require.set(target);
        return *this;
    }

    Query& Query::without(const std::uint32_t target) {
        m_reject.set(target);
        return *this;
    }

    Query& Query::any(const std::vector<std::uint32_t>& targets) {
        for (const std::uint32_t target : targets) {
            m_allow.set(target);
        }
        return *this;
    }

    bool Query::has(const std::uint32_t target) const noexcept {
        return m_require.test(target);
    }

    void Query::run(const Callback& callback, const std::vector<std::uint32_t>& targets) const {
        if (m_device) {
            if (gpu(targets)) {
                return;
            }
        }

        cpu(callback, targets);
    }

    void Query::cpu(const Callback& callback, const std::vector<std::uint32_t>& targets) const {
        std::vector<std::future<void>> tasks;

        for (const auto& archetype : m_archetypes) {
            if ((archetype->mask & m_require) != m_require) continue;
            if ((archetype->mask & m_reject).any()) continue;
            if (m_allow.any() && (archetype->mask & m_allow).none()) continue;

            tasks.push_back(std::async(std::launch::async, [pointer = archetype.get(), callback, targets]() {
                std::shared_lock lock(pointer->mutex);
                if (pointer->entities.empty()) return;

                std::vector<void*> pointers;
                pointers.reserve(targets.size());

                for (const std::uint32_t target : targets) {
                    auto item = pointer->mapping.find(target);
                    if (item == pointer->mapping.end()) break;
                    pointers.push_back(pointer->storage[item->second].data());
                }

                if (pointers.size() == targets.size()) {
                    callback(pointer->entities.size(), pointer->entities.data(), pointers);
                }
            }));
        }

        for (auto& task : tasks) {
            if (task.valid()) {
                task.get();
            }
        }
    }

    bool Query::gpu(const std::vector<std::uint32_t>& targets) const {
        (void)targets;
        return false;
    }

}