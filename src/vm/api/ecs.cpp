#include "../../../include/ecs/world.hpp"
#include "../../../include/ecs/query.hpp"
#include <lua.hpp>
#include <new>
#include <utility>
#include <vector>
#include <mutex>

using namespace voxyl;
using namespace voxyl::ecs;

struct Gpu;

template <typename T, typename U>
T* push(lua_State* L, const char* name, U&& data) {
    void* memory = lua_newuserdata(L, sizeof(T));
    new (memory) T(std::forward<U>(data));
    luaL_getmetatable(L, name);
    lua_setmetatable(L, -2);
    return static_cast<T*>(memory);
}

namespace world {
    static int create(lua_State* L) {
        push<World>(L, "world", World());
        return 1;
    }

    static int enroll(lua_State* L) {
        auto* target = static_cast<World*>(luaL_checkudata(L, 1, "world"));
        const auto size = static_cast<size_t>(luaL_checkinteger(L, 2));
        lua_pushinteger(L, target->enroll(size));
        return 1;
    }

    static int spawn(lua_State* L) {
        auto* target = static_cast<World*>(luaL_checkudata(L, 1, "world"));
        lua_pushinteger(L, target->spawn());
        return 1;
    }

    static int kill(lua_State* L) {
        auto* target = static_cast<World*>(luaL_checkudata(L, 1, "world"));
        const auto entity = static_cast<Entity>(luaL_checkinteger(L, 2));
        target->kill(entity);
        return 0;
    }

    static int attach(lua_State* L) {
        auto* target = static_cast<World*>(luaL_checkudata(L, 1, "world"));
        const auto entity = static_cast<Entity>(luaL_checkinteger(L, 2));
        const auto component = static_cast<uint32_t>(luaL_checkinteger(L, 3));
        const void* data = lua_isuserdata(L, 4) ? lua_touserdata(L, 4) : nullptr;
        void* memory = target->attach(entity, component, data);
        lua_pushlightuserdata(L, memory);
        return 1;
    }

    static int get(lua_State* L) {
        auto* target = static_cast<World*>(luaL_checkudata(L, 1, "world"));
        const auto entity = static_cast<Entity>(luaL_checkinteger(L, 2));
        const auto component = static_cast<uint32_t>(luaL_checkinteger(L, 3));
        if (void* memory = target->get(entity, component)) {
            lua_pushlightuserdata(L, memory);
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    static int has(lua_State* L) {
        const auto* target = static_cast<World*>(luaL_checkudata(L, 1, "world"));
        const auto entity = static_cast<Entity>(luaL_checkinteger(L, 2));
        const auto component = static_cast<uint32_t>(luaL_checkinteger(L, 3));
        lua_pushboolean(L, target->has(entity, component));
        return 1;
    }

    static int detach(lua_State* L) {
        auto* target = static_cast<World*>(luaL_checkudata(L, 1, "world"));
        const auto entity = static_cast<Entity>(luaL_checkinteger(L, 2));
        const auto component = static_cast<uint32_t>(luaL_checkinteger(L, 3));
        target->detach(entity, component);
        return 0;
    }

    static int fetch(lua_State* L) {
        const auto* target = static_cast<World*>(luaL_checkudata(L, 1, "world"));
        push<Query>(L, "query", target->query());
        return 1;
    }

    static int index(lua_State* L) {
        const char* key = luaL_checkstring(L, 2);
        lua_getmetatable(L, 1);
        lua_pushstring(L, key);
        lua_rawget(L, -2);
        return 1;
    }

    static int destroy(lua_State* L) {
        auto* target = static_cast<World*>(lua_touserdata(L, 1));
        target->~World();
        return 0;
    }

    static const luaL_Reg methods[] = {
        {"enroll", enroll},
        {"spawn", spawn},
        {"kill", kill},
        {"attach", attach},
        {"get", get},
        {"has", has},
        {"detach", detach},
        {"query", fetch},
        {"__index", index},
        {"__gc", destroy},
        {nullptr, nullptr}
    };
}

namespace query {
    static int with(lua_State* L) {
        auto* target = static_cast<Query*>(luaL_checkudata(L, 1, "query"));
        const auto component = static_cast<uint32_t>(luaL_checkinteger(L, 2));
        target->with(component);
        lua_pushvalue(L, 1);
        return 1;
    }

    static int without(lua_State* L) {
        auto* target = static_cast<Query*>(luaL_checkudata(L, 1, "query"));
        const auto component = static_cast<uint32_t>(luaL_checkinteger(L, 2));
        target->without(component);
        lua_pushvalue(L, 1);
        return 1;
    }

    static int any(lua_State* L) {
        auto* target = static_cast<Query*>(luaL_checkudata(L, 1, "query"));
        std::vector<uint32_t> components;
        lua_pushnil(L);
        while (lua_next(L, 2) != 0) {
            components.push_back(static_cast<uint32_t>(lua_tointeger(L, -1)));
            lua_pop(L, 1);
        }
        target->any(components);
        lua_pushvalue(L, 1);
        return 1;
    }

    static int has(lua_State* L) {
        auto* target = static_cast<Query*>(luaL_checkudata(L, 1, "query"));
        const auto component = static_cast<uint32_t>(luaL_checkinteger(L, 2));
        lua_pushboolean(L, target->has(component));
        return 1;
    }

    static int run(lua_State* L) {
        auto* target = static_cast<Query*>(luaL_checkudata(L, 1, "query"));
        luaL_checktype(L, 2, LUA_TFUNCTION);

        std::vector<uint32_t> components;
        lua_pushnil(L);
        while (lua_next(L, 3) != 0) {
            components.push_back(static_cast<uint32_t>(lua_tointeger(L, -1)));
            lua_pop(L, 1);
        }

        lua_pushvalue(L, 2);
        const int reference = luaL_ref(L, LUA_REGISTRYINDEX);

        std::mutex lua_mutex;

        target->run([L, reference, &lua_mutex](const size_t count, const Entity* entities, const std::vector<void*>& data) {
            std::lock_guard lock(lua_mutex);

            lua_rawgeti(L, LUA_REGISTRYINDEX, reference);
            lua_pushinteger(L, static_cast<lua_Integer>(count));

            lua_newtable(L);
            for (size_t iter = 0; iter < count; ++iter) {
                lua_pushinteger(L, entities[iter]);
                lua_rawseti(L, -2, static_cast<int>(iter + 1));
            }

            lua_newtable(L);
            for (size_t iter = 0; iter < data.size(); ++iter) {
                lua_pushlightuserdata(L, data[iter]);
                lua_rawseti(L, -2, static_cast<int>(iter + 1));
            }

            lua_call(L, 3, 0);
        }, components);

        luaL_unref(L, LUA_REGISTRYINDEX, reference);
        return 0;
    }

    static int index(lua_State* L) {
        const char* key = luaL_checkstring(L, 2);
        lua_getmetatable(L, 1);
        lua_pushstring(L, key);
        lua_rawget(L, -2);
        return 1;
    }

    static int destroy(lua_State* L) {
        auto* target = static_cast<Query*>(lua_touserdata(L, 1));
        target->~Query();
        return 0;
    }

    static const luaL_Reg methods[] = {
        {"with", with},
        {"without", without},
        {"any", any},
        {"has", has},
        {"run", run},
        {"__index", index},
        {"__gc", destroy},
        {nullptr, nullptr}
    };
}

extern "C" int luaopen_ecs(lua_State* L) {
    luaL_newmetatable(L, "world");
    luaL_setfuncs(L, world::methods, 0);
    lua_pop(L, 1);

    luaL_newmetatable(L, "query");
    luaL_setfuncs(L, query::methods, 0);
    lua_pop(L, 1);

    lua_newtable(L);

    lua_newtable(L);
    lua_pushcfunction(L, world::create);
    lua_setfield(L, -2, "new");

    lua_setfield(L, -2, "world");

    lua_pushvalue(L, -1);
    lua_setglobal(L, "ecs");

    return 1;
}