#ifndef _H_BASEPACKAGE
#define _H_BASEPACKAGE

#include <Platform/NString.h>

#define PKG_MANIFEST_MAX_SIZE (1024)



struct BasePackageInfo {
    std::string name;
    std::string id;
    std::string version;
};

class BasePackage
{
public:
    enum Status {
        PKG_OK = 0,
        PKG_ERR = -1
    };
    
    BasePackage() = default;
    BasePackage(BasePackageInfo *info) { m_info = *info; };
    virtual ~BasePackage() = default;

    BasePackageInfo* getInfo() { return &m_info; };
    virtual int load() { return PKG_OK; };
    virtual int exec() { return PKG_OK; };
    virtual int unload() { return PKG_OK; };

protected:
    struct BasePackageInfo m_info;
};

#endif
