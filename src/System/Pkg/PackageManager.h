#ifndef _H_PACKAGEMANAGER
#define _H_PACKAGEMANAGER

#include "BasePackage.h"
#include <unordered_map>

class PackageManager
{
public:
    PackageManager();
    ~PackageManager();

    const std::unordered_map<std::string, BasePackage *>&
        packages();

    int registerPackage(BasePackage *pkg);
    int unregisterPackage(const std::string& id);

    int runPackage(const std::string& id);

private:
    std::unordered_map<std::string, BasePackage *> m_pkgMap;
};

#endif
