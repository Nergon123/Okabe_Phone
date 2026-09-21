#pragma once
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <string>
namespace recovery {
struct ImageInfo {
    size_t size = 0;
    size_t hashOffset = 0;
    std::string project, version;
};
// Structural/checksum preflight. SHA256 and IDF verification are additional steps.
bool inspectImage(FILE* file, size_t capacity, uint16_t chip, uint16_t revision,
                  ImageInfo& info, std::string& error, void (*yield)() = nullptr);
}
