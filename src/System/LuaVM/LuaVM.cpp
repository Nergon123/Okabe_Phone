#include "LuaVM.h"
#include "LuaOkabeAPI.h"
#include "LuaOkabeLib.h"
#include <UI/Notifications.h>

static void loadLuaLib(lua_State* L, const char* name, lua_CFunction open_fn) {
    lua_pushcfunction(L, open_fn);
    lua_pushstring(L, name);
    lua_pcall(L, 1, 0, 0);
}

LuaVM::LuaVM() {
    m_luaState      = luaL_newstate();
    m_lastRunResult = 0;

    luaL_openlibs(m_luaState);
    loadLuaLib(m_luaState, OKABE_TFTLIBNAME, luaopen_tft);
    loadLuaLib(m_luaState, OKABE_SYSTEMLIBNAME, luaopen_system);
}

int LuaVM::doString(NString* str) { return 0; }

int LuaVM::doFile(IFile* file) {
    int     res = 0;
    NString errorReason;

    res             = lua_okabe_loadfile(m_luaState, file);
    m_lastRunResult = res;
    if (res) { return res; }

    res             = lua_pcall(m_luaState, 0, LUA_MULTRET, 0);
    m_lastRunResult = res;
    return res;
}

int LuaVM::doPath(NString* path) {
    int    res = 0;
    IFile* f   = VFS.open(path->c_str(), "r");

    if (f == NULL) { return LUA_ERRFILE; }

    res = doFile(f);
    delete f;

    return res;
}

void LuaVM::getErrorReason(NString* reason) {
    if (!m_lastRunResult) { return; }

    if (m_lastRunResult == LUA_ERRMEM) { *reason = NString("Not enough memory!"); }
    else {
        const char* errStr = lua_tostring(m_luaState, -1);
        if (!errStr) {
            return; // No error string or lua stack corruption
        }
        *reason = NString(errStr);
    }
}

LuaVM::~LuaVM() {
    if (m_luaState) { lua_close(m_luaState); }
}
