#pragma once
#include <cstddef>
#include <functional>
#include <string>
namespace recovery {
using DumpReader = std::function<bool(size_t, void*, size_t)>;
// Writes an exclusive temporary file, flushes, checks every byte against the
// source, then renames. Existing exports are never replaced.
bool exportDump(const std::string& path, size_t length, const DumpReader& read,
                std::string& error, void (*yield)() = nullptr);
}
