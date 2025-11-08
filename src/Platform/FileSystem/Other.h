#pragma once
#ifdef PC
#include "FileSystem.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

class StdFile : public IFile {
    std::fstream file;
    std::string  filePath;

  public:
    StdFile(const std::string& path, const char* mode) : filePath(path) {
        std::ios::openmode m = std::ios::binary;
        if (strchr(mode, 'r')) { m |= std::ios::in; }
        if (strchr(mode, 'w')) { m |= std::ios::out | std::ios::trunc; }
        if (strchr(mode, 'a')) { m |= std::ios::out | std::ios::app; }
        file.open(path, m);
    }
    size_t read(void* buf, size_t len) override {
        if (!file.read(reinterpret_cast<char*>(buf), len)) {
            return static_cast<size_t>(file.gcount());
        }
        return len;
    }

    size_t write(const void* buf, size_t len) override {
        file.write((const char*)buf, len);
        return len;
    }
    bool seek(size_t pos, int mode) override {
        (void)mode;
        file.seekg(pos, std::ios::beg);
        return file.good();
    }

    void   close() override { file.close(); }
    size_t size() override {
        auto currentPos = file.tellg();
        file.seekg(0, std::ios::end);
        size_t sz = file.tellg();
        file.seekg(currentPos);
        return sz;
    }
    bool isDirectory() override { return fs::is_directory(filePath); }
    bool available() override { return file.good() && !file.eof(); }
    void printf(const char* format, ...) override {
        char    buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        write(buffer, strlen(buffer));
    }

    std::string name() const override { return filePath; }

    void println(const char* str = "") override {
        write(str, strlen(str));
        write("\n", 1);
    }

    bool readLine(char* buffer, size_t maxLen) override {
        if (!file.good() || file.eof()) { return false; }
        file.getline(buffer, maxLen);
        return true;
    }
};

class StdFileSystem : public IFileSystem {
    std::string root;

  public:
    explicit StdFileSystem(const std::string& rootPath) : root(rootPath) {}

    bool begin() override {
        fs::create_directories(root);
        return true;
    }

    std::string makePath(const std::string& path) const {
        fs::path full = fs::path(root) / path.substr(1);
        return full.string();
    }

    bool exists(const std::string& path) override { return fs::exists(makePath(path)); }
    bool remove(const std::string& path) override { return fs::remove(makePath(path)); }
    bool mkdir(const std::string& path) override { return fs::create_directories(makePath(path)); }

    IFile* open(const std::string& path, const char* mode) override {
        return std::make_unique<StdFile>(makePath(path), mode).release();
    }

    std::vector<std::string> listDir(const std::string& path) override {
        std::vector<std::string> entries;
        fs::path                 dirPath = makePath(path);
        if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) { return entries; }
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            entries.push_back(entry.path().filename().string());
        }
        return entries;
    }
};
#endif