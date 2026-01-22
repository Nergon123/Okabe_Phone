#include <GlobalVariables.h>
enum ALLOCATION_TYPE { ALLOC_TYPE_DEFAULT = 0, ALLOC_TYPE_EXTERNAL = 1, ALLOC_TYPE_HIGH = 2 };

struct Allocation {
    size_t      size;
    const char* name;
    int         id;
    void*       ptr;
    uint32_t    caps;
    uint32_t    timestamp;
    uint8_t     type;
};

struct highalloc_t {
    int       block_ct;
    uint16_t* ptr;
};
extern std::vector<Allocation> allocations;
void freedef(void* ptr);
void* defmalloc(size_t size, const char* name, bool calloc =false , bool realloc = false, size_t n = 0);
void* extmalloc(size_t size, const char* name, bool calloc, bool realloc, size_t n = 0);
highalloc_t* _highmalloc(size_t size, const char* name, bool calloc, bool realloc);
void freehigh(highalloc_t* ptr);


size_t getTotalDefaultMemory();
size_t getFreeDefaultMemory();
size_t getTotalExternalMemory();
size_t getFreeExternalMemory();
size_t getTotalHighMemory();
size_t getFreeHighMemory();