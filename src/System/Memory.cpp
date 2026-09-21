// Here should be Memory manager and stuff
#include "Memory.h"

#ifndef PC
#include <esp_heap_caps.h>
#endif
#ifndef PC
#endif

size_t getTotalDefaultMemory() {
#ifndef PC
    return heap_caps_get_total_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
#endif
    return 0;
}
size_t getFreeDefaultMemory() {
#ifndef PC
    return heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
#endif
    return 0;
}
size_t getTotalExternalMemory() {
#ifndef PC
    return heap_caps_get_total_size(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
#endif
    return 0;
}
size_t getFreeExternalMemory() {
#ifndef PC
    return heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
#endif
    return 0;
}
size_t getTotalHighMemory() {
#ifndef PC
    return 0; // IDF 5 uses the standard capability heap.
#endif
    return 0;
}
size_t getFreeHighMemory() {
#ifndef PC
    return 0;
#endif
    return 0;
}