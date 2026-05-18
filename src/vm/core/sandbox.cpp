#include "../../../include/vm/core/sandbox.hpp"
#include <cstdio>
#include <cassert>
#include <new>

extern "C" {
#include <lualib.h>
#include <lauxlib.h>
int luaopen_ecs(lua_State* L);
}

#if defined(_WIN32)
#include <windows.h>
static void vm_restrict_resources() {
    static bool once = false;
    if (once) return;
    once = true;
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
static void vm_restrict_resources() {
    static bool once = false;
    if (once) return;
    once = true;
    rlimit limit{};
    limit.rlim_cur = 8;
    limit.rlim_max = 8;
    setrlimit(RLIMIT_NOFILE, &limit);
    limit.rlim_cur = 0;
    limit.rlim_max = 0;
    setrlimit(RLIMIT_NPROC, &limit);
}
#endif

static void vm_hook_governor(lua_State *L, const lua_Debug *debug) {
    (void)debug;
    lua_sethook(L, reinterpret_cast<lua_Hook>(vm_hook_governor), LUA_MASKCOUNT, 1);
    luaL_error(L, "sandbox: cpu budget exceeded");
}

namespace polyblox {

Sandbox::Sandbox(Context context)
    : context(context) {
    vm_restrict_resources();

    allocator = new(std::nothrow) Allocator(SANDBOX_MEMORY);
    assert(allocator != nullptr);

    state = lua_newstate(Allocator::allocator_func, allocator);
    if (state) {
        luaL_openlibs(state);

        lua_pushcfunction(state, luaopen_ecs);
        lua_pushstring(state, "ecs");
        lua_call(state, 1, 1);
        lua_setglobal(state, "ecs");
    }
}

Sandbox::~Sandbox() {
    if (state) lua_close(state);
    delete allocator;
}

int Sandbox::run(const char *path) const {
    lua_State *L = state;
    lua_State *thread = lua_newthread(L);

    lua_sethook(thread, reinterpret_cast<lua_Hook>(vm_hook_governor), LUA_MASKCOUNT, SANDBOX_INSTRUCTIONS);

    if (luaL_loadfile(thread, path) != LUA_OK) {
        std::fprintf(stderr, "sandbox error: %s\n", lua_tostring(thread, -1));
        lua_pop(L, 1);
        return 0;
    }

    if (lua_pcall(thread, 0, 0, 0) != LUA_OK) {
        std::fprintf(stderr, "runtime error: %s\n", lua_tostring(thread, -1));
        lua_pop(L, 1);
        return 0;
    }

    lua_pop(L, 1);
    return 1;
}

}