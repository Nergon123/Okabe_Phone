#ifndef _H_LUAPACKAGE
#define _H_LUAPACKAGE

#include "ZipPackage.h"
#include <System/LuaVM/LuaVM.h>

class LuaPackage : public ZipPackage {
  public:
    LuaPackage() = default;
    virtual ~LuaPackage();

    int setFile(IFile *file);
    int setFile(NString *path);

    int load();
    int exec();
    int unload();

  private:
    LuaVM *m_vm = NULL;
};

#endif
