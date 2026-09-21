#include "Std2Fs.h"
#include <cstdarg>
#include <cstring>
#include <cerrno>
#include <unistd.h>

Std2File::Std2File(const std::string& path, const char* mode)
    : _f(nullptr), _d(nullptr), _isDirectory(false), m_stat{}, m_changed(false), m_filePath(path) {
    if (!mode) mode = "r";
    if (stat(path.c_str(), &m_stat) == 0 && S_ISDIR(m_stat.st_mode)) {
        _d = opendir(path.c_str());
        _isDirectory = _d != nullptr;
    } else {
        _f = fopen(path.c_str(), mode);
        m_changed = _f != nullptr; // fopen("w") may have truncated an existing file.
    }
}
Std2File::~Std2File() { close(); }
void Std2File::updateStat() {
    if (_f) fflush(_f);
    if (stat(m_filePath.c_str(), &m_stat) == 0) m_changed = false;
}
size_t Std2File::read(void* buf, size_t len) { return _f && buf ? fread(buf, 1, len, _f) : 0; }
size_t Std2File::write(const void* buf, size_t len) {
    m_changed = true;
    return _f && buf ? fwrite(buf, 1, len, _f) : 0;
}
bool Std2File::seek(size_t pos, int mode) { return _f && fseek(_f, pos, mode) == 0; }
void Std2File::close() {
    if (_f) { fclose(_f); _f = nullptr; }
    if (_d) { closedir(_d); _d = nullptr; }
    _isDirectory = false;
}
size_t Std2File::size() {
    if (!_f) return 0;
    if (m_changed) updateStat();
    return m_stat.st_size;
}
bool Std2File::isDirectory() { return _isDirectory; }
bool Std2File::available() { return _f && !ferror(_f) && !feof(_f); }
size_t Std2File::position() {
    long pos = _f ? ftell(_f) : -1;
    return pos >= 0 ? static_cast<size_t>(pos) : 0;
}
void Std2File::printf(const char* format, ...) {
    if (!_f) return;
    va_list args;
    va_start(args, format);
    vfprintf(_f, format, args);
    va_end(args);
    m_changed = true;
}
std::string Std2File::name() const { return m_filePath.substr(m_filePath.find_last_of('/') + 1); }
std::string Std2File::path() const { return m_filePath; }
void Std2File::println(const char* str) { write(str, strlen(str)); write("\n", 1); }
bool Std2File::readLine(char* buffer, size_t maxLen) {
    if (!_f || !buffer || maxLen < 2 || !fgets(buffer, maxLen, _f)) return false;
    buffer[strcspn(buffer, "\r\n")] = 0;
    return true;
}
Std2FileSystem::Std2FileSystem(const std::string& rootPath, FileDeviceType type) : root(rootPath) {
    while (root.size() > 1 && root.back() == '/') root.pop_back();
    fstype = type;
}
std::string Std2FileSystem::makePath(const std::string& path) const {
    return root + (path.empty() || path.front() != '/' ? "/" : "") + path;
}
bool Std2FileSystem::begin() {
    struct stat st = {};
    if (stat(root.c_str(), &st) == 0) return S_ISDIR(st.st_mode);
    return ::mkdir(root.c_str(), 0755) == 0;
}
bool Std2FileSystem::exists(const std::string& path) {
    struct stat st = {};
    return stat(makePath(path).c_str(), &st) == 0;
}
bool Std2FileSystem::remove(const std::string& path) { return ::remove(makePath(path).c_str()) == 0; }
bool Std2FileSystem::mkdir(const std::string& path) { return ::mkdir(makePath(path).c_str(), 0755) == 0; }
IFile* Std2FileSystem::open(const std::string& path, const char* mode) {
    auto* file = new Std2File(makePath(path), mode);
    if (file->isOpen()) return file;
    delete file;
    return nullptr;
}
std::vector<std::string> Std2FileSystem::listDir(const std::string& path) {
    std::vector<std::string> entries;
    DIR* dir = opendir(makePath(path).c_str());
    if (!dir) return entries;
    while (auto* entry = readdir(dir)) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) entries.emplace_back(entry->d_name);
    }
    closedir(dir);
    return entries;
}
