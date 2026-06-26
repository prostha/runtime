#include "vm/api/ecs.hpp"
#include "core/ecs/world.hpp"
#include "core/ecs/find.hpp"
#include "core/primitives/vector2.hpp"
#include "core/primitives/vector3.hpp"
#include "core/primitives/vector4.hpp"
#include "core/primitives/quaternion.hpp"
#include "core/primitives/matrix3.hpp"
#include "core/primitives/matrix4.hpp"
#include "core/primitives/transform.hpp"
#include <sol/sol.hpp>
#include <unordered_map>
#include <functional>
#include <utility>
#include <vector>
#include <string>
#include <memory>

using namespace core::ecs;
using namespace core::ecs::primitives;

namespace {
    struct detail {
        std::size_t size{};
        bool object{};
        std::function<sol::object(void*, sol::this_state)> reader;
        std::function<void(void*, sol::object)> writer;
    };

    struct Entity {
        World* world = nullptr;
        Id identity = Null;

        [[nodiscard]] Id id() const { return identity; }
    };

    struct column {
        void* block;
        std::size_t size;
        std::function<sol::object(void*, sol::this_state)> reader;
        std::function<void(void*, sol::object)> writer;
        const Id* entities;
        World* world;

        [[nodiscard]] sol::object get(const sol::this_state state, const std::size_t position) const {
            auto* const pointer = static_cast<char*>(block) + (position - 1) * size;
            return reader(pointer, state);
        }

        void set(const std::size_t position, sol::object value) const {
            auto* const pointer = static_cast<char*>(block) + (position - 1) * size;
            writer(pointer, std::move(value));
        }
    };

    struct query {
        Find base;

        query& with(const uint32_t type) {
            base.with(type);
            return *this;
        }

        query& without(const uint32_t type) {
            base.without(type);
            return *this;
        }

        query& any(const sol::table& table) {
            std::vector<uint32_t> types;
            for (auto const& [key, element] : table) {
                (void)key;
                types.push_back(element.as<uint32_t>());
            }
            base.any(types);
            return *this;
        }

        [[nodiscard]] bool has(const uint32_t type) const {
            return base.has(type);
        }
    };
}

void ecs(lua_State* state) {
    sol::state_view lua(state);
    sol::table hidden = lua.create_table();

    const auto store = std::make_shared<std::unordered_map<uint32_t, detail>>();

    hidden.new_usertype<column>("column",
        sol::no_constructor,
        sol::meta_function::index, [](const sol::this_state state, const column& self, const sol::object& key) -> sol::object {
            if (key.is<std::size_t>()) {
                return self.get(state, key.as<std::size_t>());
            }
            if (key.is<std::string>()) {
                const std::string name = key.as<std::string>();
                if (name == "get") return sol::make_object(state, &column::get);
                if (name == "set") return sol::make_object(state, &column::set);
                if (name == "index") {
                    return sol::make_object(state, [](const column& self, const std::size_t position) {
                        return Entity{ self.world, self.entities[position - 1] };
                    });
                }
            }
            return sol::lua_nil;
        },
        sol::meta_function::new_index, [](const column& self, const std::size_t position, sol::object value) {
            self.set(position, std::move(value));
        }
    );

    hidden.new_usertype<query>("query",
        sol::no_constructor,
        "with", &query::with,
        "without", &query::without,
        "any", &query::any,
        "has", &query::has
    );

    hidden.new_usertype<Entity>("Entity",
        sol::no_constructor,
        "id", &Entity::id,
        "set", [store](const sol::this_state state, Entity& self, const uint32_t type, const sol::object& value) -> Entity& {
            if (!self.world) return self;
            if (const auto match = store->find(type); match != store->end()) {
                const auto& item = match->second;
                if (item.size == 0) {
                    self.world->set(self.identity, type, nullptr);
                } else {
                    std::vector<char> memory(item.size, 0);
                    item.writer(memory.data(), value);
                    self.world->set(self.identity, type, memory.data());
                }
            }
            return self;
        },
        "tag", [](Entity& self, const uint32_t type) -> Entity& {
            if (self.world) {
                self.world->set(self.identity, type, nullptr);
            }
            return self;
        },
        "attach", [](Entity& self, const Entity& parent) -> Entity& {
            if (self.world) {
                self.world->attach(self.identity, parent.identity);
            }
            return self;
        },
        "parent", [](const sol::this_state state, const Entity& self) -> sol::object {
            if (!self.world) return sol::lua_nil;
            const Id leader = self.world->parent(self.identity);
            if (leader == Null) return sol::lua_nil;
            return sol::make_object(state, Entity{ self.world, leader });
        }
    );

    sol::usertype<World> world_type = lua.new_usertype<World>("World",
        sol::no_constructor,
        "component", [store](const sol::this_state state, World& instance, const std::string& name, const sol::optional<sol::object>& value) {
            (void)state;
            detail item;
            item.object = false;

            if (value && value->valid() && !value->is<sol::lua_nil_t>()) {
                const auto& input = *value;
                if (input.is<Vector2>()) {
                    item.size = sizeof(Vector2);
                    item.reader = [](void* pointer, const sol::this_state state) { return sol::make_object(state, static_cast<Vector2*>(pointer)); };
                    item.writer = [](void* pointer, const sol::object &object) { *static_cast<Vector2*>(pointer) = object.as<Vector2>(); };
                } else if (input.is<Vector3>()) {
                    item.size = sizeof(Vector3);
                    item.reader = [](void* pointer, const sol::this_state state) { return sol::make_object(state, static_cast<Vector3*>(pointer)); };
                    item.writer = [](void* pointer, const sol::object& object) { *static_cast<Vector3*>(pointer) = object.as<Vector3>(); };
                } else if (input.is<Vector4>()) {
                    item.size = sizeof(Vector4);
                    item.reader = [](void* pointer, const sol::this_state state) { return sol::make_object(state, static_cast<Vector4*>(pointer)); };
                    item.writer = [](void* pointer, const sol::object& object) { *static_cast<Vector4*>(pointer) = object.as<Vector4>(); };
                } else if (input.is<Quaternion>()) {
                    item.size = sizeof(Quaternion);
                    item.reader = [](void* pointer, const sol::this_state state) { return sol::make_object(state, static_cast<Quaternion*>(pointer)); };
                    item.writer = [](void* pointer, const sol::object& object) { *static_cast<Quaternion*>(pointer) = object.as<Quaternion>(); };
                } else if (input.is<Matrix3>()) {
                    item.size = sizeof(Matrix3);
                    item.reader = [](void* pointer, const sol::this_state state) { return sol::make_object(state, static_cast<Matrix3*>(pointer)); };
                    item.writer = [](void* pointer, const sol::object& object) { *static_cast<Matrix3*>(pointer) = object.as<Matrix3>(); };
                } else if (input.is<Matrix4>()) {
                    item.size = sizeof(Matrix4);
                    item.reader = [](void* pointer, const sol::this_state state) { return sol::make_object(state, static_cast<Matrix4*>(pointer)); };
                    item.writer = [](void* pointer, const sol::object &object) { *static_cast<Matrix4*>(pointer) = object.as<Matrix4>(); };
                } else if (input.is<Transform>()) {
                    item.size = sizeof(Transform);
                    item.reader = [](void* pointer, const sol::this_state state) { return sol::make_object(state, static_cast<Transform*>(pointer)); };
                    item.writer = [](void* pointer, const sol::object &object) { *static_cast<Transform*>(pointer) = object.as<Transform>(); };
                } else {
                    item.size = sizeof(int);
                    item.object = true;
                    item.reader = [](void* pointer, const sol::this_state state) {
                        const int reference = *static_cast<int*>(pointer);
                        lua_rawgeti(state, LUA_REGISTRYINDEX, reference);
                        sol::object object(state, -1);
                        lua_pop(state, 1);
                        return object;
                    };
                    item.writer = [](void* pointer, const sol::object& object) {
                        int* cell = static_cast<int*>(pointer);
                        lua_State* current = object.lua_state();
                        if (*cell > 0) {
                            luaL_unref(current, LUA_REGISTRYINDEX, *cell);
                        }
                        *cell = luaL_ref(current, LUA_REGISTRYINDEX);
                    };
                }
            } else {
                item.size = 0;
                item.reader = [](const void* pointer, const sol::this_state state) { (void)pointer; (void)state; return sol::object(sol::lua_nil); };
                item.writer = [](const void* pointer, const sol::object &object) { (void)pointer; (void)object; };
            }
            const uint32_t type = instance.component(name, item.size);
            (*store)[type] = item;
            return type;
        },
        "spawn", [](World& instance) {
            return Entity{ &instance, instance.spawn() };
        },
        "clone", [](World& instance, const Entity& entity) {
            return Entity{ &instance, instance.clone(entity.identity) };
        },
        "dispose", [](World& instance, const sol::object& entity) {
            const Id identity = entity.is<Entity>() ? entity.as<Entity>().identity : entity.as<Id>();
            instance.dispose(identity);
        },
        "clear", [](World& instance) {
            instance.clear();
        },
        "has", [](const World& instance, const sol::object& entity, const uint32_t type) {
            const Id identity = entity.is<Entity>() ? entity.as<Entity>().identity : entity.as<Id>();
            return instance.has(identity, type);
        },
        "remove", [](World& instance, const sol::object& entity, const uint32_t type) {
            const Id identity = entity.is<Entity>() ? entity.as<Entity>().identity : entity.as<Id>();
            instance.remove(identity, type);
        },
        "query", [](const World& instance) { return query{ instance.query() }; },
        "batch", [](World& instance, sol::function action) {
            instance.batch([action = std::move(action)]() { (void)action(); });
        },
        "set", [store](World& instance, const sol::object& entity, const uint32_t type, const sol::object& value) {
            const Id identity = entity.is<Entity>() ? entity.as<Entity>().identity : entity.as<Id>();
            const auto match = store->find(type);
            if (match == store->end()) return;
            const auto& item = match->second;
            if (item.size == 0) {
                instance.set(identity, type, nullptr);
                return;
            }
            std::vector<char> memory(item.size, 0);
            item.writer(memory.data(), value);
            instance.set(identity, type, memory.data());
        },
        "get", [store](const sol::this_state state, const World& instance, const sol::object& entity, const uint32_t type) -> sol::object {
            const Id identity = entity.is<Entity>() ? entity.as<Entity>().identity : entity.as<Id>();
            auto* const pointer = instance.get(identity, type);
            if (!pointer) return sol::lua_nil;
            const auto match = store->find(type);
            if (match == store->end()) return sol::lua_nil;
            const auto& item = match->second;
            return item.reader(pointer, state);
        },
        "wait", [](World& instance, const uint32_t type, sol::function action) {
            instance.wait(type, [action = std::move(action)](const Id identity) { (void)action(identity); });
        },
        "loop", [store](const sol::this_state state, const World& instance, const query& search, sol::function callback) {
            if (!callback.valid()) return;
            auto* const world = const_cast<World*>(&instance);

            instance.loop(search.base, [state, callback = std::move(callback), store, world, order = search.base.plan()](const std::size_t count, const Id* const entities, const std::vector<void*>& blocks) {
                std::vector<sol::object> arguments;
                arguments.reserve(blocks.size() + 1);
                arguments.push_back(sol::make_object(state, count));
                for (std::size_t index = 0; index < blocks.size(); ++index) {
                    if (index < order.size()) {
                        const uint32_t type = order[index];
                        if (const auto match = store->find(type); match != store->end()) {
                            const auto& item = match->second;
                            arguments.push_back(sol::make_object(state, column{ blocks[index], item.size, item.reader, item.writer, entities, world }));
                            continue;
                        }
                    }
                    arguments.emplace_back(sol::lua_nil);
                }
                (void)callback.call(sol::as_args(arguments));
            });
        }
    );

    world_type["new"] = []() {
        return std::make_unique<World>();
    };
}