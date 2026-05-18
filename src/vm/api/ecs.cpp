#include "../../../include/vm/api/api.hpp"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int luaopen_ecs(lua_State* L) {
    return 0;
}
}