#ifndef _H_ZIP_FILE_PROVIDER
#define _H_ZIP_FILE_PROVIDER

#include <GlobalVariables.h>
#include <Platform/FileSystem/FileSystem.h>
#include "unzipLIB.h"

class ZipFileProvider : public IFile, UNZIP
{
public:
    ~ZipFileProvider();

    int    setFile(NString *path);
    int    setFile(const std::string &path);
    int    openZip(IFile *file);
    unz_file_info* getFileInfo();

    size_t read(void* buf, size_t len) override;
    size_t write(const void* buf, size_t len) override { return 0; };
    bool   seek(size_t pos, int mode = SEEK_CUR) override { return false; };
    void   close() override;
    size_t size() override;
    bool   isDirectory() override { return false; };
    bool   available() override;
    size_t position() override;

    std::string name() const override { return ""; };
    void printf(const char* format, ...) override { (void *)format; };
    void println(const char* str = "") override { (void *)str; };
    bool readLine(char* buffer, size_t maxLen) override { return false; };

private:
    bool m_isFileOpen = false;
    bool m_isZipOpen = false;
    size_t m_fileSize = 0;
    size_t m_filePos = 0;

    unz_file_info m_fileInfo;
    
};

#endif
