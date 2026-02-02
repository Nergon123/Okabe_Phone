#ifndef _H_LUA_OKABE_LIB
#define _H_LUA_OKABE_LIB

#ifdef __cplusplus
extern "C" {
#endif

#include "lua/lua.h"

#define OKABE_TFTLIBNAME "tft"
int luaopen_tft(lua_State *L);

#define OKABE_SYSTEMLIBNAME "system"
int luaopen_system(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif
