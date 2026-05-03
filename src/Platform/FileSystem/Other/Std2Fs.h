#pragma once
#ifdef PC
#include "../FileSystem.h"
#include <Defines.h>

extern "C" {
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
}
// namespace fs = std::filesystem;

class Std2File : public IFile {
  FILE *_f;
  DIR *_d;
  bool _isDirectory;
  struct stat m_stat;
  bool m_changed;
  std::string  m_filePath;
  // size_t pos;

  void updateStat();

  public:
    Std2File(const std::string& path, const char* mode);
    virtual ~Std2File();
    
    size_t read(void* buf, size_t len) override;
    size_t write(const void* buf, size_t len) override;
    bool seek(size_t pos, int mode) override;

    void   close() override;
    size_t size() override;
    
    bool   isDirectory() override;
    bool   available() override;
    size_t position() override;

    void printf(const char* format, ...) override;

    std::string name() const override;
    std::string path() const override;

    void println(const char* str = "") override;

    bool readLine(char* buffer, size_t maxLen) override;
};

class Std2FileSystem : public IFileSystem {
    std::string    root;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-private-field"
    FileDeviceType fstype;
#pragma GCC diagnostic pop

  public:
    explicit Std2FileSystem(const std::string& rootPath, FileDeviceType type);

    std::string makePath(const std::string& path) const;
  
    bool begin() override;

    bool exists(const std::string& path) override;
    bool remove(const std::string& path) override;
    bool mkdir(const std::string& path) override;

    IFile* open(const std::string& path, const char* mode) override;

    std::vector<std::string> listDir(const std::string& path) override;
};
#endif
