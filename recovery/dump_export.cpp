#include "dump_export.h"
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
namespace recovery {
bool exportDump(const std::string& path, size_t length, const DumpReader& read,
                std::string& error, void (*yield)()) {
    struct stat st;
    if (!length) { error = "Empty dump"; return false; }
    if (stat(path.c_str(), &st) == 0 || errno != ENOENT) { error = "Destination exists or cannot be checked"; return false; }
    // Keep temporary names 8.3-compatible too (the board's FAT configuration
    // need not enable long filenames). Raw and summary exports are sequential.
    size_t dot = path.find_last_of('.');
    size_t slash = path.find_last_of('/');
    const std::string temporary = path.substr(0, dot != std::string::npos &&
        (slash == std::string::npos || dot > slash) ? dot : path.size()) + ".tmp";
    if (temporary == path) { error = "Reserved temporary filename"; return false; }
    int fd = open(temporary.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0600);
    if (fd < 0) { error = "Cannot create export: " + std::string(strerror(errno)); return false; }
    bool ok = true;
    unsigned char bytes[1024], actual[1024];
    for (size_t offset = 0; ok && offset < length;) {
        size_t n = std::min(length - offset, sizeof(bytes));
        if (!read(offset, bytes, n)) { error = "Dump read failed"; ok = false; break; }
        size_t written = 0;
        while (written < n) {
            ssize_t count = write(fd, bytes + written, n - written);
            if (count < 0 && errno == EINTR) continue;
            if (count <= 0) { error = "Export write failed (full or removed storage)"; ok = false; break; }
            written += size_t(count);
        }
        offset += n;
        if (yield) yield();
    }
    if (ok && fsync(fd) != 0) { error = "Export flush failed"; ok = false; }
    if (close(fd) != 0 && ok) { error = "Export close failed"; ok = false; }
    FILE* check = ok ? fopen(temporary.c_str(), "rb") : nullptr;
    if (ok && !check) { error = "Cannot reopen export"; ok = false; }
    for (size_t offset = 0; ok && offset < length;) {
        size_t n = std::min(length - offset, sizeof(bytes));
        if (!read(offset, bytes, n) || fread(actual, 1, n, check) != n || memcmp(bytes, actual, n)) {
            error = "Export read-back verification failed"; ok = false; break;
        }
        offset += n;
        if (yield) yield();
    }
    if (check) fclose(check);
    if (ok) {
        // This application is the only writer, but still recheck before rename.
        if (stat(path.c_str(), &st) == 0 || errno != ENOENT || rename(temporary.c_str(), path.c_str()) != 0) {
            error = "Cannot publish verified export"; ok = false;
        }
    }
    if (!ok) unlink(temporary.c_str());
    return ok;
}
}
