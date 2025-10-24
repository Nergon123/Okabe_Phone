// ProgmemCompat.h
#pragma once
#include <stdint.h>

#ifdef ARDUINO
  #include <pgmspace.h>
  #define READ_BYTE(p)      pgm_read_byte(p)
  #define READ_WORD(p)      pgm_read_word(p)
  #define READ_DWORD(p)     pgm_read_dword(p)
  #define READ_PTR(p)       (typeof(p))pgm_read_dword(p)
  #define MEMCPY_P(dest, src, n) memcpy_P(dest, src, n)
#else
  #define READ_BYTE(p)      (*(const uint8_t*)(p))
  #define READ_WORD(p)      (*(const uint16_t*)(p))
  #define READ_DWORD(p)     (*(const uint32_t*)(p))
  #define READ_PTR(p)       (p)
  #define MEMCPY_P(dest, src, n) memcpy(dest, src, n)
#endif
