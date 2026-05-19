#include "../include/vm/core/sandbox.hpp"
#include <cstdio>
#include <cassert>
#include <new>
#include <stdexcept>

extern "C" {
#include <lualib.h>
#include <lauxlib.h>
int luaopen_ecs(lua_State* L);
int luaopen_math(lua_State* L);
}

#if defined(_WIN32)
#include <windows.h>
static void confine() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    HANDLE job = CreateJobObject(nullptr, nullptr);
    if (!job) return;
    JOBOBJECT_BASIC_LIMIT_INFORMATION limits = {};
    limits.LimitFlags = JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
    limits.ActiveProcessLimit = 1;
    SetInformationJobObject(job, JobObjectBasicLimitInformation, &limits, sizeof(limits));
    AssignProcessToJobObject(job, GetCurrentProcess());
}
#else
#include <sys/resource.h>
static void confine() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    rlimit limit{};
    limit.rlim_cur = 8;
    limit.rlim_max = 8;
    setrlimit(RLIMIT_NOFILE, &limit);
    limit.rlim_cur = 0;
    limit.rlim_max = 0;
    setrlimit(RLIMIT_NPROC, &limit);
}
#endif

static void govern(lua_State *state, const lua_Debug *debug) {
    (void)debug;
    lua_sethook(state, reinterpret_cast<lua_Hook>(govern), LUA_MASKCOUNT, 1);
    luaL_error(state, "sandbox: cpu budget exceeded");
}

namespace sandbox {

Sandbox::Sandbox() {
    try {
        confine();

        memory = new(std::nothrow) allocator::Allocator(MEMORY);
        assert(memory != nullptr);

        state = lua_newstate(allocator::Allocator::manage, memory);
        if (!state) {
            throw std::runtime_error("Failed to initialize Lua state.");
        }

        luaL_openlibs(state);

        lua_getglobal(state, "package");
        if (lua_istable(state, -1)) {
            lua_getfield(state, -1, "preload");
            if (lua_istable(state, -1)) {

                lua_pushcfunction(state, luaopen_ecs);
                lua_setfield(state, -2, "voxyl.ecs");

                lua_pushcfunction(state, luaopen_math);
                lua_setfield(state, -2, "voxyl.math");
            }
            lua_pop(state, 1);
        }
        lua_pop(state, 1);
    }
    catch (const std::exception& exception) {
        std::fprintf(stderr, "sandbox initialization error: %s\n", exception.what());

        if (state) {
            lua_close(state);
            state = nullptr;
        }
        delete memory;
        memory = nullptr;
    }
    catch (...) {
        std::fprintf(stderr, "sandbox initialization error: Unknown exception caught.\n");
        if (state) {
            lua_close(state);
            state = nullptr;
        }
        delete memory;
        memory = nullptr;
    }
}

Sandbox::~Sandbox() {
    if (state) lua_close(state);
    delete memory;
}

int Sandbox::run(const char *path) const {
    try {
        lua_State *context = state;
        if (!context) {
            std::fprintf(stderr, "sandbox error: Lua state is uninitialized\n");
            return 0;
        }

        lua_State *thread = lua_newthread(context);
        if (!thread) {
            std::fprintf(stderr, "sandbox error: Failed to create Lua coroutine thread\n");
            return 0;
        }

        lua_sethook(thread, reinterpret_cast<lua_Hook>(govern), LUA_MASKCOUNT, INSTRUCTIONS);

        if (luaL_loadfile(thread, path) != LUA_OK) {
            std::fprintf(stderr, "sandbox error: %s\n", lua_tostring(thread, -1));
            lua_pop(context, 1);
            return 0;
        }

        if (lua_pcall(thread, 0, 0, 0) != LUA_OK) {
            std::fprintf(stderr, "runtime error: %s\n", lua_tostring(thread, -1));
            lua_pop(context, 1);
            return 0;
        }

        lua_pop(context, 1);
        return 1;
    }
    catch (const std::exception& exception) {
        std::fprintf(stderr, "unexpected C++ exception in sandbox run: %s\n", exception.what());
        return 0;
    }
    catch (...) {
        std::fprintf(stderr, "unexpected unknown exception in sandbox run\n");
        if (state) {
            lua_settop(state, 0);
        }
        return 0;
    }
}

}