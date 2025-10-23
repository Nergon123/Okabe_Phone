#pragma once
#include <string>
#include <vector>

class IFile {
  public:
    virtual ~IFile()                                     = default;
    virtual size_t read(void* buf, size_t len)           = 0;
    virtual size_t write(const void* buf, size_t len)    = 0;
    virtual bool   seek(size_t pos, int mode = SEEK_CUR) = 0;
    virtual void   close()                               = 0;
    virtual size_t size()                                = 0;
    virtual bool   isDirectory()                         = 0;
    virtual bool   available()                           = 0;
    virtual std::string name() const                        = 0;
    virtual void printf(const char* format, ...)        = 0;
    virtual void println(const char* str = "")          = 0;
    virtual bool readLine(char* buffer, size_t maxLen) = 0;
};

class IFileSystem {
  public:
    virtual ~IFileSystem()                                                           = default;
    virtual bool                     begin()                                         = 0;
    virtual bool                     exists(const std::string& path)                 = 0;
    virtual bool                     remove(const std::string& path)                 = 0;
    virtual bool                     mkdir(const std::string& path)                  = 0;
    virtual IFile*                   open(const std::string& path, const char* mode) = 0;
    virtual std::vector<std::string> listDir(const std::string& path)                = 0;
};
