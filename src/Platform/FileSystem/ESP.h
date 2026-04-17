#pragma once
#include "FS.h"
#include "FileSystem.h"
#include "SD.h"
#include "SPIFFS.h"

class Esp32File : public IFile {
    File    file;
    fs::FS* fs;

  public:
    Esp32File(File f, fs::FS* filesystem) : file(f), fs(filesystem) {}
    Esp32File() : file(File()) {}
    size_t read(void* buf, size_t len) override { return file.read((uint8_t*)buf, len); }
    size_t write(const void* buf, size_t len) override {
        return file.write((const uint8_t*)buf, len);
    }
    bool        seek(size_t pos, int mode) override { return file.seek(pos, (fs::SeekMode)mode); }
    size_t      position() override { return file.position(); }
    void        close() override { file.close(); }
    size_t      size() override { return file.size(); }
    bool        isDirectory() override { return file.isDirectory(); }
    bool        available() override { return file.available(); }
    std::string name() const override { return std::string(file.name()); }
    std::string path() const override {
        if (!fs) { return file.path(); }
        //hacks i guess...
        if (fs == &SPIFFS) { return "/spiffs" + std::string(file.path()); }
        else if (fs == &SD) { return "/sd" + std::string(file.path()); }
        return std::string(file.path());
    }
    void printf(const char* format, ...) override {
        char    buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        file.write((const uint8_t*)buffer, strlen(buffer));
    }

    void println(const char* str = "") override { file.println(str); }

    bool readLine(char* buffer, size_t maxLen) override {
        size_t idx = 0;
        while (idx < maxLen - 1 && file.available()) {
            char c = file.read();
            if (c == '\n') {
                buffer[idx] = '\0';
                return true;
            }
            if (c != '\r') { buffer[idx++] = c; }
        }
        buffer[idx] = '\0';
        return idx > 0;
    }
};

class Esp32FileSystem : public IFileSystem {
    fs::FS*        fs;
    FileDeviceType fstype;

  public:
    Esp32FileSystem(fs::FS* fsImpl, FileDeviceType type) : fs(fsImpl), fstype(type) {}

    bool begin() override { return false; }
    bool exists(const std::string& path) override { return fs->exists(path.c_str()); }
    bool remove(const std::string& path) override { return fs->remove(path.c_str()); }
    bool mkdir(const std::string& path) override { return fs->mkdir(path.c_str()); }

    IFile* open(const std::string& path, const char* mode) override {
        File f = fs->open(path.c_str(), mode);
        return f ? new Esp32File(f, fs) : nullptr;
    }

    std::vector<std::string> listDir(const std::string& path) override {
        std::vector<std::string> entries;
        File                     dir = fs->open(path.c_str());
        if (!dir || !dir.isDirectory()) { return entries; }
        File f = dir.openNextFile();
        while (f) {
            entries.push_back(f.name());
            f = dir.openNextFile();
        }
        return entries;
    }
};
