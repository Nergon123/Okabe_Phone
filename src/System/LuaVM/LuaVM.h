#ifndef _H_LUAVM
#define _H_LUAVM

#include <GlobalVariables.h>
#include "lua/lua.hpp"
#include "../../Platform/NString.h"
#include <Platform/FileSystem/VFS.h>

class LuaVM 
{
 public:
	LuaVM();
	~LuaVM();
	
	int doString(NString* str);
	int doFile(IFile* file);
	int doPath(NString* path);
	void getErrorReason(NString* reason);

 private:
 	lua_State* m_luaState;
 	int m_lastRunResult;
};

#endif
