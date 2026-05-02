#include "Std2Fs.h"
// #include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <filesystem>
namespace fs = std::filesystem;


const static char TAG[] = "Std2Fs";

#define log_e(format, ...) ESP_LOGE(TAG, format, ##__VA_ARGS__)
#define STD2FILE_LINE_MAX  1024

void Std2File::updateStat()
{
    if (!stat(m_filePath.c_str(), &m_stat)) {
        m_changed = false;
    }
}

Std2File::Std2File(const std::string& path, const char* mode)
    : m_filePath(path), _f(NULL), _d(NULL), _isDirectory(false)
    , m_changed(false)
{
    // size_t tempLen = strlen(m_filePath.c_str());
    // char *temp = (char *)malloc(tempLen);

    const char *temp = m_filePath.c_str();

    // ESP_LOGD(TAG, "open: f=%s, mode=%s", m_filePath.c_str(), mode);
    
    if (!mode || mode[0] == 'r') {
        if (!stat(temp, &m_stat)) {
            //file found
            if (S_ISREG(m_stat.st_mode)) {
                _isDirectory = false;
                _f = fopen(temp, mode);
                if (!_f) {
                    log_e("fopen(%s) failed", temp);
                }
                // if (_f && (_stat.st_blksize == 0)) {
                //     setvbuf(_f, NULL, _IOFBF, DEFAULT_FILE_BUFFER_SIZE);
                // }
            } else if (S_ISDIR(m_stat.st_mode)) {
                _isDirectory = true;
                _d = opendir(temp);
                if (!_d) {
                    log_e("opendir(%s) failed", temp);
                }
            } else {
                // log_e("Unknown type 0x%08" PRIX32 " for file %s", (uint32_t)((_stat.st_mode) & _IFMT), temp);
                log_e("Unknown type (%i) for file %s", (uint32_t)((m_stat.st_mode)), temp);
            }
        } else {
            //file not found
            //try to open as directory
            _d = opendir(temp);
            if (_d) {
                _isDirectory = true;
            } else {
                _isDirectory = false;
                log_e("dir stat(%s) failed", temp);
            }
        }
    } else {
        //lets create this new file
        _isDirectory = false;
        _f = fopen(temp, mode);
        if (!_f) {
            log_e("fopen(%s) failed", temp);
        }
        if (!stat(temp, &m_stat)) {
            // if (_f && (_stat.st_blksize == 0)) {
            //     setvbuf(_f, NULL, _IOFBF, DEFAULT_FILE_BUFFER_SIZE);
            // }
        } else {
            log_e("stat(%s) failed", temp);
        }
    }

    // ESP_LOGD(TAG, "open: _f=%p, is_dir=%i", _f, _isDirectory);
    
}

Std2File::~Std2File()
{
    Std2File::close();
}

size_t Std2File::read(void* buf, size_t len)
{
    // ESP_LOGD(TAG, "read: buf=%p, _f=%p, is_dir=%i, len=%li", buf, _f, _isDirectory, len);
    if (_isDirectory || !_f || !buf || !len) {
        return 0;
    }
    // ESP_LOGD(TAG, "read call");

  return fread(buf, 1, len, _f);
}

size_t Std2File::write(const void* buf, size_t len)
{
    if (_isDirectory || !_f || !buf || !len) {
        return 0;
    }

    m_changed = true;
    return fwrite(buf, 1, len, _f);
}
bool Std2File::seek(size_t pos, int mode)
{
    // ESP_LOGD(TAG, "seek(%s): pos=%li, mpde=%i, _f=%p, is_dir=%i",m_filePath.c_str(), pos, mode, _f, _isDirectory);
    if (_isDirectory || !_f) {
        return false;
    }

    // ESP_LOGD(TAG, "seek(%s): check ok!!",m_filePath.c_str());
    
    return fseek(_f, pos, mode) == 0;
}

void Std2File::close()
{
    if (_isDirectory && _d) {
        closedir(_d);
        _d = NULL;
        _isDirectory = false;
    } else if (_f) {
        fclose(_f);
        _f = NULL;
    }
}

size_t Std2File::size()
{
    if (_isDirectory || !_f) {
        return 0;
    }
    
    if (m_changed) {
        updateStat();
    }
    
    return m_stat.st_size;
}

bool Std2File::isDirectory()
{
    return _isDirectory;
}

bool Std2File::available()
{
    if (_isDirectory || !_f) {
        return false;
    }
    
    return (ferror(_f) && !feof(_f));
}
size_t Std2File::position()
{
    if (_isDirectory || !_f) {
        return 0;
    }

    return ftell(_f);
}

void Std2File::printf(const char* format, ...)
{
    if (_isDirectory || !_f) {
        return;
    }
    
    char    buffer[STD2FILE_LINE_MAX];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Std2File::write(buffer, strlen(buffer));
}

std::string Std2File::name() const
{
    return m_filePath;
}

std::string Std2File::path() const
{
    return m_filePath;
}

void Std2File::println(const char* str)
{
    if (_isDirectory || !_f) {
        return;
    }

    int len = strlen(str);
    
    if (len > 0) {
        write(str, len);
    }
    
    write("\n", 1);
}

bool Std2File::readLine(char* buffer, size_t maxLen)
{
    if (_isDirectory || !_f || Std2File::available()) {
        return false;
    }

    char *str = fgets(buffer, maxLen, _f);

    // force bool type
    return (!!str);
}


std::string Std2FileSystem::makePath(const std::string& path) const {
        fs::path full = fs::path(root) / path.substr(1);
        return full.string();
}


Std2FileSystem::Std2FileSystem(const std::string& rootPath, FileDeviceType type)
    : root(rootPath), fstype(type)
{}


bool Std2FileSystem::begin()
{
    fs::create_directories(root);
    return true;
}

bool Std2FileSystem::exists(const std::string& path)
{
    return fs::exists(makePath(path));
}
bool Std2FileSystem::remove(const std::string& path)
{
    return fs::remove(makePath(path));
}

bool Std2FileSystem::mkdir(const std::string& path)
{
    return fs::create_directory(makePath(path));
}

IFile* Std2FileSystem::open(const std::string& path, const char* mode)
{
    return std::make_unique<Std2File>(makePath(path), mode).release();
}

std::vector<std::string> Std2FileSystem::listDir(const std::string& path)
{
    std::vector<std::string> entries;
    fs::path                 dirPath = makePath(path);
    
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        return entries;
    }
    
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        entries.push_back(entry.path().filename().string());
    }
    return entries;
}
