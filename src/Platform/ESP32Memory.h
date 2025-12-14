#pragma once

#ifdef PC
#include <cstdlib>

inline bool psramFound() {
    return true;
}

inline void* ps_malloc(size_t size) {
    return malloc(size);
}
#else
#include <esp_system.h>
#endif