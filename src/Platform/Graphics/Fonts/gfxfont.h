// Adopted by Bodmer to support TFT_eSPI library.

#include <Defines.h>
#include <stdint.h>
#ifndef _GFXFONT_H_
#define _GFXFONT_H_

typedef struct {               // Data stored PER GLYPH
    uint32_t bitmapOffset;     // Pointer into GFXfont->bitmap
    uint8_t  width, height;    // Bitmap dimensions in pixels
    uint8_t  xAdvance;         // Distance to advance cursor (x axis)
    int8_t   xOffset, yOffset; // Dist from cursor pos to UL corner
} GFXglyph;

typedef struct {           // Data stored for FONT AS A WHOLE:
    uint8_t  *bitmap;      // Glyph bitmaps, concatenated
    GFXglyph *glyph;       // Glyph array
    uint16_t  first, last; // ASCII extents
    uint8_t   yAdvance;    // Newline distance (y axis)
} GFXfont;

#pragma pack(push, 1)
typedef struct {
    char    headerName[8];
    uint8_t version;
    uint8_t count;
} fontsHeader;

typedef struct {               // Data stored PER GLYPH
    uint32_t bitmapOffset;     // Pointer into GFXfont->bitmap
    uint8_t  width, height;    // Bitmap dimensions in pixels
    uint8_t  xAdvance;         // Distance to advance cursor (x axis)
    int8_t   xOffset, yOffset; // Dist from cursor pos to UL corner
} GFXglyphPacked;

typedef struct {          // Data stored for FONT AS A WHOLE:
    uint32_t bitmap;      // Glyph bitmaps, concatenated
    uint32_t glyph;       // Glyph array
    uint16_t first, last; // ASCII extents
    uint8_t  yAdvance;    // Newline distance (y axis)
    uint32_t bitmapSize;
} GFXfontPacked;
#pragma pack(pop)

#include "FreeMono9pt7b.h"
#include "FreeMono9pt8bCyr.h"
#include "FreeSans12pt7b.h"
#include "FreeSans12pt8bCyr.h"
#include "FreeSans9pt7b.h"
#include "FreeSans9pt8bCyr.h"
#include "FreeSansBold9pt7b.h"
#include "FreeSansBold9pt8bCyr.h"
#endif // _GFXFONT_H_
