#pragma once
#include <map>
#include <string>
#include "FileSystem.h"
#ifdef PC
#include "Other.h"
#else
#include "ESP.h"
#endif
class VirtualFileSystem : public IFileSystem {
    struct MountPoint {
        std::string prefix;
        IFileSystem* fs;
    };
    std::vector<MountPoint> mounts;

    // Helper: find which FS handles a given path
    IFileSystem* findFS(const std::string& path, std::string& subPath) {
        for (auto& m : mounts) {
            if (path.rfind(m.prefix, 0) == 0) { // prefix match at start
                subPath = path.substr(m.prefix.size());
                if (subPath.empty()) subPath = "/";
                return m.fs;
            }
        }
        return nullptr;
    }

public:
    void mount(const std::string& prefix, IFileSystem* fs) {
        mounts.push_back({ prefix, fs });
    }

    bool begin() override {
        bool ok = true;
        for (auto& m : mounts)
            ok &= m.fs->begin();
        return ok;
    }

    bool exists(const std::string& path) override {
        std::string sub;
        IFileSystem* fs = findFS(path, sub);
        return fs ? fs->exists(sub) : false;
    }

    bool remove(const std::string& path) override {
        std::string sub;
        IFileSystem* fs = findFS(path, sub);
        return fs ? fs->remove(sub) : false;
    }

    bool mkdir(const std::string& path) override {
        std::string sub;
        IFileSystem* fs = findFS(path, sub);
        return fs ? fs->mkdir(sub) : false;
    }

    IFile* open(const std::string& path, const char* mode = "r") override {
        std::string sub;
        IFileSystem* fs = findFS(path, sub);
        return fs ? fs->open(sub, mode) : nullptr;
    }
    std::vector<std::string> listDir(const std::string& path) override {
        std::string sub;
        IFileSystem* fs = findFS(path, sub);
        return fs ? fs->listDir(sub) : std::vector<std::string>();
    }
};

using NFile = IFile;
extern VirtualFileSystem VFS;