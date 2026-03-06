#include "UTF.h"
const char *utf8_decode(const char *s, uint32_t *out){
    uint8_t b0 = (uint8_t)s[0];

    if (b0 < 0x80) {
        *out = b0;
        return s + 1;
    }
    if ((b0 & 0xE0) == 0xC0) {
        *out = ((b0 & 0x1F) << 6) | ((uint8_t)s[1] & 0x3F);
        return s + 2;
    }
    if ((b0 & 0xF0) == 0xE0) {
        *out = ((b0 & 0x0F) << 12) | (((uint8_t)s[1] & 0x3F) << 6) | ((uint8_t)s[2] & 0x3F);
        return s + 3;
    }
    if ((b0 & 0xF8) == 0xF0) {
        *out = ((b0 & 0x07) << 18) | (((uint8_t)s[1] & 0x3F) << 12) |
               (((uint8_t)s[2] & 0x3F) << 6) | ((uint8_t)s[3] & 0x3F);
        return s + 4;
    }

    *out = 0xFFFD; // replacement char
    return s + 1;
}

char *utf8_encode(uint32_t cp, char *out) {
    if (cp <= 0x7F) {
        out[0] = cp;
        return out + 1;
    }
    else if (cp <= 0x7FF) {
        out[0] = 0xC0 | (cp >> 6);
        out[1] = 0x80 | (cp & 0x3F);
        return out + 2;
    }
    else if (cp <= 0xFFFF) {
        out[0] = 0xE0 | (cp >> 12);
        out[1] = 0x80 | ((cp >> 6) & 0x3F);
        out[2] = 0x80 | (cp & 0x3F);
        return out + 3;
    }
    else {
        out[0] = 0xF0 | (cp >> 18);
        out[1] = 0x80 | ((cp >> 12) & 0x3F);
        out[2] = 0x80 | ((cp >> 6) & 0x3F);
        out[3] = 0x80 | (cp & 0x3F);
        return out + 4;
    }
}