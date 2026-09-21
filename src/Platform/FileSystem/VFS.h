#pragma once
#include "FileSystem.h"
#include <cstring>
#include <string>
#include "Other/Std2Fs.h"
class VirtualFileSystem : public IFileSystem {
    // Mount roots belong to the virtual namespace. SPIFFS, for example, can
    // list its root even when stat/open cannot provide a real directory handle.
    class DirectoryEntry : public IFile {
        std::string entryPath;
        bool opened = true;
      public:
        explicit DirectoryEntry(const std::string& path) : entryPath(path) {}
        size_t read(void*, size_t) override { return 0; }
        size_t write(const void*, size_t) override { return 0; }
        bool seek(size_t, int) override { return false; }
        size_t position() override { return 0; }
        void close() override { opened = false; }
        size_t size() override { return 0; }
        bool isDirectory() override { return opened; }
        bool available() override { return false; }
        std::string name() const override {
            return entryPath == "/" ? "/" : entryPath.substr(entryPath.find_last_of('/') + 1);
        }
        std::string path() const override { return entryPath; }
        void printf(const char*, ...) override {}
        void println(const char*) override {}
        bool readLine(char*, size_t) override { return false; }
    };
    static std::string directoryPath(std::string path) {
        while (path.size() > 1 && path.back() == '/') path.pop_back();
        return path;
    }
    bool isVirtualDirectory(const std::string& path) const {
        const std::string canonical = directoryPath(path);
        if (canonical == "/") return true;
        for (const auto& mount : mounts) if (mount.prefix == canonical) return true;
        return false;
    }

    struct MountPoint {
        std::string  prefix;
        IFileSystem* fs;
    };
    std::vector<MountPoint> mounts;

    // Helper: find which FS handles a given path
    IFileSystem* findFS(const std::string& path, std::string& subPath) {
        for (auto& m : mounts) {
            if (path.rfind(m.prefix, 0) == 0 &&
                (path.size() == m.prefix.size() || path[m.prefix.size()] == '/')) { // prefix match at start
                subPath = path.substr(m.prefix.size());
                if (subPath.empty()) { subPath = "/"; }
                return m.fs;
            }
        }
        return nullptr;
    }

  public:
    void mount(const std::string& prefix, IFileSystem* fs) { mounts.push_back({directoryPath(prefix), fs}); }

    bool begin() override {
        bool ok = true;
        for (auto& m : mounts) { ok &= m.fs->begin(); }
        return ok;
    }

    bool exists(const std::string& path) override {
        std::string sub;

        if (isVirtualDirectory(path)) return true;

        IFileSystem* fs = findFS(path, sub);
        return fs ? fs->exists(sub) : false;
    }

    bool remove(const std::string& path) override {
        if (isVirtualDirectory(path)) return false;
        std::string  sub;
        IFileSystem* fs = findFS(path, sub);
        return fs ? fs->remove(sub) : false;
    }

    bool mkdir(const std::string& path) override {
        if (isVirtualDirectory(path)) return false;
        std::string  sub;
        IFileSystem* fs = findFS(path, sub);
        return fs ? fs->mkdir(sub) : false;
    }

    IFile* open(const std::string& path, const char* mode = "r") override {
        if (isVirtualDirectory(path)) {
            if (mode && strcmp(mode, "r") && strcmp(mode, "rb")) return nullptr;
            return new DirectoryEntry(directoryPath(path));
        }
        std::string  sub;
        IFileSystem* fs = findFS(path, sub);
        return fs ? fs->open(sub, mode) : nullptr;
    }
    std::vector<std::string> listDir(const std::string& path) override {
        std::string              sub;
        std::vector<std::string> dirs;
        if (path.compare("/") == 0) {
            for (MountPoint mnt : mounts) { dirs.push_back(mnt.prefix.c_str() + 1); }
            return dirs;
        }
        IFileSystem* fs = findFS(path, sub);
        return fs ? fs->listDir(sub) : std::vector<std::string>();
    }
};

using NFile = IFile;
extern VirtualFileSystem VFS;
