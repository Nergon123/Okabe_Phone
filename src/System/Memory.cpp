// Here should be Memory manager and stuff
#include "Memory.h"

#ifdef IDF_VER
#include <esp_heap_caps.h>
#include <esp32/himem.h>
#define ESP32_MEMORY
#endif
#ifndef PC
#endif

std::vector<Allocation> allocations;

void removeFromAllocations(void* ptr) {
    for (size_t i = 0; i < allocations.size(); i++) {
        if (allocations[i].ptr == ptr) {
            allocations.erase(allocations.begin() + i);
            break;
        }
    }
}
void freedef(void* ptr) {
    removeFromAllocations(ptr);
    free(ptr);
}

#ifndef ESP32_MEMORY
void* pcmalloc(size_t n, size_t size, const char* name, uint8_t type, bool calloc, bool realloc) {
    int   id  = allocations.size();
    void* ptr = nullptr;
    if (calloc) { ptr = calloc(n, size); }
    else if (realloc) { ptr = realloc(ptr, size); }
    else { ptr = malloc(size); }
    if (ptr == nullptr) {
        ESP_LOGE("MEMORY", "Failed to allocate %d bytes for %s (id %d)", size, name, id);
    }
    else { allocations.push_back({size, name, id, ptr, 0, hw->millis(), type}); }
}
#endif
void* defmalloc(size_t n, size_t size, const char* name, bool calloc, bool realloc) {
    (void)name;
    int   id  = allocations.size();
    void* ptr = nullptr;
#ifdef ESP32_MEMORY
    ptr = heap_caps_malloc(size, MALLOC_CAP_8BIT);
    if (ptr != nullptr) {
        allocations.push_back(
            {size, name, id, ptr, MALLOC_CAP_8BIT, hw->millis(), ALLOC_TYPE_DEFAULT});
    }
    else { ESP_LOGE("MEMORY", "Failed to allocate %d bytes for %s (id %d)", size, name, id); }
#else
    ptr = pcmalloc(n, size, name, ALLOC_TYPE_DEFAULT, calloc, realloc);
#endif
    return ptr;
}

// SPIRAM allocations can be freed using normal free()
void freeext(void* ptr) { freedef(ptr); }
// for external memory allocations
void* extmalloc(size_t size, const char* name, bool calloc, bool realloc, size_t n) {
    (void)name;
    int   id  = allocations.size();
    void* ptr = nullptr;
#ifdef ESP32_MEMORY
    if (calloc) { ptr = heap_caps_calloc(n, size / n, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM); }
    else if (realloc) { ptr = heap_caps_realloc(ptr, size, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM); }
    else { ptr = heap_caps_malloc(size, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM); }

    if (ptr == nullptr) {
        ESP_LOGE("MEMORY", "Failed to allocate %d bytes for %s (id %d)", size, name, id);
    }
    else {
        allocations.push_back({size, name, id, ptr, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM,
                               hw->millis(), ALLOC_TYPE_EXTERNAL});
    }
#else
    ptr = pcmalloc(n, size, name, ALLOC_TYPE_EXTERNAL, calloc, realloc);
#endif
    return ptr;
}

void highfree(highalloc_t* handle) {
#ifdef ESP32_MEMORY
    if (esp_himem_free((esp_himem_handle_t)handle) != ESP_OK) {
        ESP_LOGE("MEMORY", "Failed to free high memory allocation %p", handle);
        return;
    }
    removeFromAllocations(handle);
    delete handle;
#else
    if (handle->ptr) { free(handle->ptr); }
    removeFromAllocations(handle);
    delete handle;
#endif
}

#define HIGHALLOC_BLOCK_SIZE (32 * 1024)
// extmalloc only allows to allocate first 4MB in PSRAM on ESP32, so for higher allocations use
// highmalloc, be aware that it allocates memory in 32KB blocks
highalloc_t* _highmalloc(size_t size, const char* name, bool calloc, bool realloc) {
    (void)name;
    int          id  = allocations.size();
    highalloc_t* ptr = new highalloc_t{0, nullptr};
    if (size % HIGHALLOC_BLOCK_SIZE != 0) {
        size += HIGHALLOC_BLOCK_SIZE - (size % HIGHALLOC_BLOCK_SIZE);
    }
    ptr->block_ct = size / HIGHALLOC_BLOCK_SIZE;
#ifdef ESP32_MEMORY

    if (esp_himem_alloc(size, (esp_himem_handle_t*)ptr) != ESP_OK) {
        ESP_LOGE("MEMORY", "Failed to allocate %d bytes for %s (id %d)", size, name, id);
        ptr->ptr = nullptr;
    }
    if (ptr != nullptr) {
        allocations.push_back({size, name, id, ptr, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL,
                               hw->millis(), ALLOC_TYPE_HIGH});
    }
#else
    // this probably works diffrently then i thinking, read
    // https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/himem.html
    
    ptr->ptr = (uint16_t*)pcmalloc(ptr->block_ct, HIGHALLOC_BLOCK_SIZE, name, ALLOC_TYPE_HIGH,
                                   calloc, realloc);
#endif
    return ptr;
}

size_t getTotalDefaultMemory() {
#ifdef ESP32_MEMORY
    return heap_caps_get_total_size(MALLOC_CAP_8BIT);
#endif
    return 0;
}
size_t getFreeDefaultMemory() {
#ifdef ESP32_MEMORY
    return heap_caps_get_free_size(MALLOC_CAP_8BIT);
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