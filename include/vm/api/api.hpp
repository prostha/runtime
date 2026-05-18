#ifndef VM_API_H
#define VM_API_H

#ifdef __cplusplus
#include <lua.hpp>
extern "C" {
#else
#include <lua.h>
#endif

int luaopen_ecs(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif // VM_API_H