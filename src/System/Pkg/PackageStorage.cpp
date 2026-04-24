#include "PackageStorage.h"
#include "LuaPackage.h"
#include <GlobalVariables.h>
#include <Platform/FileSystem/FileSystem.h>

static const char TAG[] = "PackageStorage";

static const char PKG_FORMAT[] = ".lpkg";

static int copyFile(const std::string &src, const std::string &dest) {
    char   buf[1024];
    IFile *srcFile;
    IFile *destFile;
    size_t bytesToCopy;
    size_t count;

    srcFile = VFS.open(src, "r");
    if (!srcFile) { return -1; }

    destFile = VFS.open(dest, "w");
    if (!destFile) { return -1; }

    ESP_LOGD(TAG, "Copy \"%s\" to \"%s\"", src.c_str(), dest.c_str());

    bytesToCopy = srcFile->size();

    while (bytesToCopy > 0) {
        count = srcFile->read(buf, sizeof(buf));
        destFile->write(buf, count);

        bytesToCopy -= count;

        ESP_LOGD(TAG, "copy %zd, left %zd bytes", count, bytesToCopy);
    }

    delete srcFile;
    delete destFile;

    return 0;
}

int PackageStorage::init() {
    int                      res  = 0;
    std::vector<std::string> list = VFS.listDir(PKG_STORAGE_DIR);

    ESP_LOGD(TAG, "Items in dir: %zd", list.size());

    LuaPackage *pkg;
    NString     nstr;
    for (auto fname : list) {
        if (!NString(fname).endsWith(PKG_FORMAT)) { continue; }

        ESP_LOGD(TAG, "Attempting to register package: %s", fname.c_str());
        pkg  = new LuaPackage();
        nstr = NString(PKG_STORAGE_DIR) + NString(fname);
        res  = pkg->setFile(&nstr);
        ESP_LOGD(TAG, "Pkg parse result %i", res);
        if (res == 0) { pm.registerPackage(pkg); }
        else { delete pkg; }
    }

    return 0;
}

int PackageStorage::installPackage(const std::string &path) {
    int         res = -1;
    size_t      pos = 0;
    NString     npath(path);
    NString     fname;
    NString     storagePath(PKG_STORAGE_DIR);
    LuaPackage *pkg;

    ESP_LOGD(TAG, "Attempting to install: %s", npath.c_str());

    if (!npath.endsWith(PKG_FORMAT)) { goto exit; }

    // validate pkg
    pkg = new LuaPackage();
    res = pkg->setFile(&npath);
    delete pkg;
    if (res < 0) {
        ESP_LOGD(TAG, "Package parse error: %i", res);
        goto exit;
    }

    pos   = npath.lastIndexOf('/');
    fname = npath.substring(pos, path.length());
    storagePath += fname;

    if (VFS.exists(storagePath)) {
        ESP_LOGD(TAG, "File exists in storage (%s), skip install", storagePath.c_str());
        res = -1;
        goto exit;
    }

    res = copyFile(path, storagePath);

    if (res == 0) {
        pkg = new LuaPackage();
        pkg->setFile(&npath); // checked before copy, should not fail
        pm.registerPackage(pkg);
        // don't delete pkg, it's managed by PackageManager
    }

exit:
    ESP_LOGD(TAG, "Install result: %i", res);
    return res;
}

int PackageStorage::removePackage(const std::string &id) {
    int                      res  = 0;
    std::vector<std::string> list = VFS.listDir(PKG_STORAGE_DIR);

    ESP_LOGD(TAG, "Items in dir: %zd", list.size());
    ESP_LOGD(TAG, "Id to remove: %s", id.c_str());

    ZipPackage      *pkg;
    NString          nstr;
    BasePackageInfo *info;
    for (auto fname : list) {
        if (!NString(fname).endsWith(PKG_FORMAT)) { continue; }

        pkg  = new ZipPackage();
        nstr = NString(PKG_STORAGE_DIR) + NString(fname);
        res  = pkg->setZip(&nstr);

        if (res == 0) {
            info = pkg->getInfo();
            ESP_LOGD(TAG, "%s: %s", info->id.c_str(), fname.c_str());
            if (info->id == id) {
                ESP_LOGD(TAG, "Removing %s", fname.c_str());
                VFS.remove(nstr);
                pm.unregisterPackage(id);
                delete pkg;
                return 0;
            }
        }

        delete pkg;
    }

    return -1;
}
