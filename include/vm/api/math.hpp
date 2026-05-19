#pragma once
#ifdef __cplusplus
#include <lua.hpp>
extern "C" {
#else
#include <lua.h>
#endif

int luaopen_math(lua_State *L);

#ifdef __cplusplus
}
#endif