#ifndef _H_LOKABEAPI
#define _H_LOKABEAPI

#ifdef __cplusplus
extern "C" {
#endif

#include "lua/lua.h"

int lua_okabe_print(lua_State *L);
int lua_okabe_loadpath(lua_State *L, const char *filename);
int lua_okabe_readable(const char *filename);

#ifdef __cplusplus
}
#endif

#endif
