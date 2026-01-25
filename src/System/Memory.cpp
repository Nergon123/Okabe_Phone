// Here should be Memory manager and stuff
#include "Memory.h"

#ifdef IDF_VER
#include <esp_heap_caps.h>
#include <esp32/himem.h>
#define ESP32_MEMORY
#endif
#ifndef PC
#endif

size_t getTotalDefaultMemory() {
#ifdef ESP32_MEMORY
    return heap_caps_get_total_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
#endif
    return 0;
}
size_t getFreeDefaultMemory() {
#ifdef ESP32_MEMORY
    return heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
#endif
    return 0;
}
size_t getTotalExternalMemory() {
#ifdef ESP32_MEMORY
    return heap_caps_get_total_size(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
#endif
    return 0;
}
size_t getFreeExternalMemory() {
#ifdef ESP32_MEMORY
    return heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
#endif
    return 0;
}
size_t getTotalHighMemory() {
#ifdef ESP32_MEMORY
    return esp_himem_get_phys_size();
#endif
    return 0;
}
size_t getFreeHighMemory() {
#ifdef ESP32_MEMORY
    return esp_himem_get_free_size();
#endif
    return 0;
}