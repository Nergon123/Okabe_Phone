#include <string>
#ifndef _H_PACKAGESTORAGE
#define _H_PACKAGESTORAGE

#define PKG_STORAGE_DIR "/spiffs/system/pkg/"

class PackageStorage {
  public:
    enum Status {
        PACKAGESTORAGE_OK       = 0,
        PACKAGESTORAGE_ERR      = -1,
        PACKAGESTORAGE_NOTFOUND = -2,
        PACKAGESTORAGE_EXIST    = -3
    };

    PackageStorage()  = default;
    ~PackageStorage() = default;

    int init();
    int installPackage(const std::string& path);
    int removePackage(const std::string& id);
};

#endif
