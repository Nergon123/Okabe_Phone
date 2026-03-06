#include <stdint.h>
const char *utf8_decode(const char *s, uint32_t *out);
char *utf8_encode(uint32_t cp, char *out);