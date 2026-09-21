#ifndef PC
#include "esp_memory.h"
#include <esp_heap_caps.h>
bool psramFound() { return heap_caps_get_total_size(MALLOC_CAP_SPIRAM) != 0; }
void* ps_malloc(size_t size) {
    void* p = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    return p ? p : heap_caps_malloc(size, MALLOC_CAP_8BIT);
}
void* ps_realloc(void* ptr, size_t size) {
    if (!size) { heap_caps_free(ptr); return nullptr; }
    void* p = heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    return p ? p : heap_caps_realloc(ptr, size, MALLOC_CAP_8BIT);
}
#endif
