#include "image_check.h"
#include <algorithm>
#include <cstring>
namespace recovery {
namespace {
uint32_t le32(const unsigned char* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}
uint16_t le16(const unsigned char* p) { return p[0] | (p[1] << 8); }
std::string field(const unsigned char* p, size_t n) {
    const char* s = reinterpret_cast<const char*>(p);
    const void* end = memchr(s, 0, n);
    return std::string(s, end ? static_cast<const char*>(end) - s : n);
}
}
bool inspectImage(FILE* f, size_t capacity, uint16_t chip, uint16_t revision,
                  ImageInfo& info, std::string& error, void (*yield)()) {
    info = {};
    auto fail = [&](const char* reason) { error = reason; return false; };
    if (fseek(f, 0, SEEK_END) != 0) return fail("Cannot seek firmware file");
    long length = ftell(f);
    if (length < 24 + 8 + 256 || static_cast<size_t>(length) > capacity)
        return fail("Image is empty, too small or too large");
    info.size = static_cast<size_t>(length);
    rewind(f);
    unsigned char header[24];
    if (fread(header, 1, sizeof(header), f) != sizeof(header)) return fail("Cannot read header");
    if (header[0] != 0xe9 || header[1] == 0 || header[1] > 16 || header[23] > 1)
        return fail("Not an ESP application image");
    if (le16(header + 12) != chip) return fail("Firmware targets a different ESP chip");
    unsigned minimum = le16(header + 15);
    if (!minimum) minimum = unsigned(header[14]) * 100;
    unsigned maximum = le16(header + 17);
    if (revision < minimum || (maximum && maximum != 65535 && revision > maximum))
        return fail("Firmware does not support this chip revision");
    unsigned char checksum = 0xef;
    size_t position = sizeof(header);
    unsigned char buffer[1024];
    for (unsigned segment = 0; segment < header[1]; ++segment) {
        unsigned char sh[8];
        if (info.size - position < sizeof(sh) || fread(sh, 1, sizeof(sh), f) != sizeof(sh))
            return fail("Truncated segment header");
        position += sizeof(sh);
        size_t remaining = le32(sh + 4);
        if (remaining > info.size - position || remaining % 4)
            return fail("Invalid segment length");
        if (segment == 0 && remaining < 256) return fail("Missing application descriptor");
        bool first = segment == 0;
        while (remaining) {
            size_t count = std::min(remaining, sizeof(buffer));
            if (fread(buffer, 1, count, f) != count) return fail("Truncated segment data");
            if (first) {
                if (le32(buffer) != 0xabcd5432) return fail("Not an app-only firmware.bin");
                info.version = field(buffer + 16, 32);
                info.project = field(buffer + 48, 32);
                first = false;
            }
            for (size_t i = 0; i < count; ++i) checksum ^= buffer[i];
            remaining -= count;
            position += count;
            if (yield) yield();
        }
    }
    size_t checksumOffset = position | size_t(15);
    if (checksumOffset >= info.size || fseek(f, checksumOffset, SEEK_SET) != 0)
        return fail("Missing image checksum");
    if (fgetc(f) != checksum) return fail("Image checksum mismatch");
    size_t end = checksumOffset + 1;
    if (header[23]) {
        if (info.size - end < 32) return fail("Missing SHA256 digest");
        info.hashOffset = end;
        end += 32;
    }
    // Signed images carry additional blocks. This updater deliberately accepts
    // only plain app binaries; IDF still validates the completed OTA image.
    if (end != info.size) return fail("Trailing data or signed image unsupported");
    rewind(f);
    return true;
}
}
