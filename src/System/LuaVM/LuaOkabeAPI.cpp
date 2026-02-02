// Misc api exported to lua (for example to replace os lib)
// c-side declaration in "lokabelib.h"

#include <GlobalVariables.h>
#include "lua/lua.hpp"

typedef struct LoadNF {
    int extraline;
    IFile *f;
    char buff[LUAL_BUFFERSIZE];
} LoadNF;

static const char *getNF (lua_State *L, void *ud, size_t *size)
{
    LoadNF *lf = (LoadNF *)ud;
    (void)L;
    if (lf->extraline) {
     lf->extraline = 0;
     *size = 1;
     return "\n";
    }
    if (!lf->f->available()) return NULL;
    *size = lf->f->read(lf->buff, sizeof(lf->buff));
    return (*size > 0) ? lf->buff : NULL;
}

int lua_okabe_loadfile (lua_State *L, IFile *file) {
    LoadNF lf;
    int status, readstatus;
    int c;
    int fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
    lf.extraline = 0;

    lua_pushfstring(L, "@%s", file->name().c_str());
    lf.f = file;
    //   lf.f = VFS.open(NString(filename), "r");
    //   if (lf.f == NULL) return LUA_ERRFILE;//errfile(L, "open", fnameindex);
    // }
    //  c = getc(lf.f);
    //  if (c == '#') {  /* Unix exec. file? */
    //    lf.extraline = 1;
    //    while ((c = getc(lf.f)) != EOF && c != '\n') ;  /* skip first line */
    //    if (c == '\n') c = getc(lf.f);
    // }
    //  if (c == LUA_SIGNATURE[0] && filename) {  /* binary file? */
    //    lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
    //    if (lf.f == NULL) return errfile(L, "reopen", fnameindex);
    //    /* skip eventual `#!...' */
    //   while ((c = getc(lf.f)) != EOF && c != LUA_SIGNATURE[0]) ;
    //    lf.extraline = 0;
    //  }
    //  ungetc(c, lf.f);
    status = lua_load(L, getNF, &lf, lua_tostring(L, -1));
    //readstatus = ferror(lf.f);
    lf.f->close();  /* close file (even in case of errors) */
    //  if (readstatus) {
    //    lua_settop(L, fnameindex);  /* ignore results from `lua_load' */
    //    return errfile(L, "read", fnameindex);
    //  }
    lua_remove(L, fnameindex);
    return status;
}


extern "C" {
	int lua_okabe_print (lua_State *L) {
	  int n = lua_gettop(L);  /* number of arguments */
	  int i;
	  lua_getglobal(L, "tostring");
	  for (i=1; i<=n; i++) {
	    const char *s;
	    lua_pushvalue(L, -1);  /* function to be called */
	    lua_pushvalue(L, i);   /* value to print */
	    lua_call(L, 1, 1);
	    s = lua_tostring(L, -1);  /* get result */
	    if (s == NULL)
	      return luaL_error(L, LUA_QL("tostring") " must return a string to "
	                           LUA_QL("print"));
	    if (i>1) ESP_LOGI("LuaVM", "\t");
	    ESP_LOGI("LuaVM", "%s", s);
	    lua_pop(L, 1);  /* pop result */
	  }
	  ESP_LOGI("LuaVM", "\n");
	  return 0;
	}
		
    int lua_okabe_loadpath (lua_State *L, const char *filename)
    {
     //    if (filename == NULL) {
	    //     return LUA_ERRFILE;
	    // }
	    IFile *f = VFS.open(NString(filename), "r");
	    if (f == NULL) {
	        return LUA_ERRFILE;
	    }
	    lua_okabe_loadfile(L, f);
	    delete f;
    }

	int lua_okabe_readable(const char *filename)
	{
		NFile* f = VFS.open(NString(filename), "r");
		if(f == NULL) return 0;
		f->close();
		return 1;		
	}
}
