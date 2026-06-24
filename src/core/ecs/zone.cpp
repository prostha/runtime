#include "../include/core/ecs/zone.hpp"
#include <cstring>
#include <mutex>
#include <shared_mutex>

namespace core {

    std::uint32_t Zone::type(const std::string& name, std::size_t size) {
        if (const auto find = names.find(name); find != names.end()) {
            return find->second;
        }
        const auto id = static_cast<std::uint32_t>(sizes.size());
        sizes.push_back(size);
        names[name] = id;

        if (name == "parent") {
            head = id;
        } else if (name == "children") {
            tail = id;
        }
        return id;
    }

    Id Zone::spawn() {
        Id id;
        if (!free.empty()) {
            id = free.back();
            free.pop_back();
        } else {
            id = next++;
        }
        if (id >= slots.size()) {
            slots.resize(id + 1);
        }
        Mask blank;
        Kind* arch = view(blank);
        arch->push(id);
        slots[id] = {arch, arch->rows.size() - 1};
        return id;
    }

    Id Zone::clone(Id id) {
        const Id duplicate = spawn();
        Slot source = slots[id];
        Kind* arch = source.type;
        std::shared_lock lock_shared(arch->lock);

        const Mask mask = arch->mask;
        move(duplicate, mask);

        Slot destination = slots[duplicate];
        Kind* goal = destination.type;
        std::unique_lock lock_unique(goal->lock);

        for (auto const& [tag, track] : arch->maps) {
            if (const std::size_t size = sizes[tag]; size > 0) {
                const std::size_t offset = goal->maps[tag];
                std::memcpy(goal->page[offset].data() + destination.row * size,
                            arch->page[track].data() + source.row * size,
                            size);
            }
        }
        return duplicate;
    }

    void Zone::kill(Id id) {
        if (lock) {
            steps.emplace_back([this, id]() { kill(id); });
            return;
        }
        Slot record = slots[id];
        if (!record.type) return;

        if (tail != 0xFFFFFFFF && has(id, tail)) {
            if (const auto* list = static_cast<std::vector<Id>*>(get(id, tail))) {
                for (const Id nested : *list) {
                    kill(nested);
                }
            }
        }

        if (const std::uint32_t moved = record.type->pull(record.row); moved != id) {
            slots[moved].row = record.row;
        }
        slots[id] = {nullptr, 0};
        free.push_back(id);
    }

    void Zone::wipe() {
        kinds.clear();
        slots.clear();
        free.clear();
        cache.clear();
        events.clear();
        steps.clear();
        next = 0;
        lock = false;
    }

    void Zone::attach(const Id entity, const Id parent) {
        if (head == 0xFFFFFFFF) type("parent", sizeof(Id));
        if (tail == 0xFFFFFFFF) type("children", sizeof(std::vector<Id>));

        set(entity, head, &parent);

        if (!has(parent, tail)) {
            std::vector<Id> list;
            list.push_back(entity);
            set(parent, tail, &list);
        } else {
            auto* list = static_cast<std::vector<Id>*>(get(parent, tail));
            list->push_back(entity);
        }
    }

    Id Zone::parent(Id id) const {
        if (head == 0xFFFFFFFF || !has(id, head)) return Null;
        return *static_cast<const Id*>(const_cast<Zone*>(this)->get(id, head));
    }

    const std::vector<Id>& Zone::children(Id id) const {
        static constexpr std::vector<Id> empty;
        if (tail == 0xFFFFFFFF || !has(id, tail)) return empty;
        return *static_cast<const std::vector<Id>*>(this->get(id, tail));
    }

    void* Zone::set(Id id, std::uint32_t type, const void* data) {
        if (lock) {
            steps.emplace_back([this, id, type, data]() { set(id, type, data); });
            return nullptr;
        }
        const Kind* arch = slots[id].type;
        Mask mask = arch->mask;
        mask.set(type);

        move(id, mask);

        void* pointer = get(id, type);
        if (data && pointer) {
            std::memcpy(pointer, data, sizes[type]);
        }

        if (const auto find = events.find(type); find != events.end()) {
            find->second(id);
        }
        return pointer;
    }

    void* Zone::get(const Id id, const std::uint32_t type) const {
        Slot record = slots[id];
        Kind* arch = record.type;
        if (!arch) return nullptr;

        const auto find = arch->maps.find(type);
        if (find == arch->maps.end()) return nullptr;

        const std::size_t size = sizes[type];
        if (size == 0) return nullptr;
        return arch->page[find->second].data() + record.row * size;
    }

    bool Zone::has(const Id id, const std::uint32_t type) const {
        Slot record = slots[id];
        if (!record.type) return false;
        return record.type->mask.test(type);
    }

    void Zone::remove(Id id, std::uint32_t type) {
        if (lock) {
            steps.emplace_back([this, id, type]() { remove(id, type); });
            return;
        }
        Slot record = slots[id];
        const Kind* arch = record.type;
        Mask mask = arch->mask;
        mask.reset(type);
        move(id, mask);
    }

    void Zone::global(const std::uint32_t type, const void* data, const std::size_t size) {
        auto& resource = cache[type];
        resource.resize(size);
        if (data && size > 0) {
            std::memcpy(resource.data(), data, size);
        }
    }

    void* Zone::global(const std::uint32_t type) {
        const auto find = cache.find(type);
        if (find == cache.end()) return nullptr;
        return find->second.data();
    }

    void Zone::watch(const std::uint32_t type, const Task& action) {
        events[type] = action;
    }

    void Zone::batch(const std::function<void()>& flow) {
        lock = true;
        flow();
        lock = false;
        for (const auto& task : steps) {
            task();
        }
        steps.clear();
    }

    Find Zone::query() const {
        return Find(kinds);
    }

    void Zone::loop(const Find& query, const Find::Call& action) const {
        query.process(action, query.plan());
    }

    Kind* Zone::view(const Mask& mask) {
        for (const auto& arch : kinds) {
            if (arch->mask == mask) return arch.get();
        }
        auto arch = std::make_unique<Kind>();
        arch->mask = mask;
        for (std::uint32_t tag = 0; tag < Cap; ++tag) {
            if (mask.test(tag)) {
                arch->maps[tag] = arch->page.size();
                arch->page.emplace_back();
                arch->bits.push_back(sizes[tag]);
            }
        }

        kinds.push_back(std::move(arch));
        return kinds.back().get();
    }

    void Zone::move(const Id id, const Mask& mask) {
        Slot source = slots[id];
        Kind* old = source.type;
        if (old->mask == mask) return;

        Kind* goal = view(mask);
        goal->push(id);

        const Slot destination = {goal, goal->rows.size() - 1};

        for (auto const& [tag, track] : old->maps) {
            if (mask.test(tag)) {
                if (const std::size_t size = sizes[tag]; size > 0) {
                    const std::size_t offset = goal->maps[tag];
                    std::memcpy(goal->page[offset].data() + destination.row * size,
                                old->page[track].data() + source.row * size,
                                size);
                }
            }
        }

        if (const std::uint32_t moved = old->pull(source.row); moved != id) {
            slots[moved].row = source.row;
        }
        slots[id] = destination;
    }

}