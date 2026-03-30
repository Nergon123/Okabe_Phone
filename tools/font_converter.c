/*
 * Font Converter - Converts Adafruit GFX font arrays to ADAFONT binary format
 *
 * Compile with:
 *   gcc -o font_converter font_converter.c
 *
 * Usage:
 *   ./font_converter output.nfnt
 *
 * Add your font headers and modify the fonts array below
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ArrSize(a) (sizeof(a) / sizeof(a[0]))

/* ===== Font Data Structures ===== */

typedef struct {
    uint32_t bitmapOffset;
    uint8_t  width, height;
    uint8_t  xAdvance;
    int8_t   xOffset, yOffset;
} GFXglyph;

typedef struct {
    uint8_t  *bitmap;
    GFXglyph *glyph;
    uint16_t  first, last;
    uint8_t   yAdvance;
} GFXfont;

#pragma pack(push, 1)
typedef struct {
    char    headerName[8];
    uint8_t version;
    uint8_t count;
} fontsHeader;

typedef struct {
    uint32_t bitmapOffset;
    uint8_t  width, height;
    uint8_t  xAdvance;
    int8_t   xOffset, yOffset;
} GFXglyphPacked;

typedef struct {
    uint32_t bitmap;
    uint32_t glyph;
    uint16_t first, last;
    uint8_t  yAdvance;
    uint32_t bitmapSize;
} GFXfontPacked;
#pragma pack(pop)

/* ===== Include Your Fonts Here ===== */
/* Example:
 * #include "FreeSans12pt7b.h"
 * #include "JP.h"
 */

/* PLACEHOLDER: Define sample fonts for testing */
/* In real usage, include actual Adafruit GFX font headers */
/* You would include real fonts here */
/* For now, create a simple test setup */
#define TEST_FONTS 1

#if TEST_FONTS
/* You would include real fonts here */
GFXfont fonts[] = {
    /* Add your fonts here */
    JP_P1, JP_P2, JP_P3};

size_t bitmapArraySizes[] = {
    /* Add bitmap sizes here */
    ArrSize(JP_BitmapsP1), ArrSize(JP_BitmapsP2), ArrSize(JP_BitmapsP3)};
#else
/* Fallback: create an empty but valid file structure */
GFXfont fonts[]            = {};
size_t  bitmapArraySizes[] = {};
#endif

/* ===== Converter Logic ===== */

int main(int argc, char *argv[]) {
    const char *outputFile = (argc > 1) ? argv[1] : "output.nfnt";

    FILE *output = fopen(outputFile, "wb");
    if (!output) {
        printf("Error: Cannot open %s for writing\n", outputFile);
        return 1;
    }

    size_t fontCount = ArrSize(fonts);

    if (fontCount == 0) {
        printf(
            "Error: No fonts defined. Add your font arrays to fonts[] and bitmapArraySizes[]\n");
        printf("Example:\n");
        printf("  #include \"FreeSans12pt7b.h\"\n");
        printf("  #include \"JP.h\"\n");
        printf("  GFXfont fonts[] = {FreeSans12pt7b, JP};\n");
        printf("  size_t bitmapArraySizes[] = {ArrSize(FreeSans12pt7b_Bitmap), "
               "ArrSize(JP_Bitmap)};\n");
        fclose(output);
        return 1;
    }

    /* Write header */
    fontsHeader header;
    strncpy(header.headerName, "ADAFONT", 8);
    header.version = 1;
    header.count   = fontCount;

    if (fwrite(&header, sizeof(header), 1, output) != 1) {
        printf("Error: Failed to write header\n");
        fclose(output);
        return 1;
    }

    printf("Converting %zu font(s) to %s\n", fontCount, outputFile);

    /* Process each font */
    for (size_t fnt = 0; fnt < fontCount; fnt++) {
        size_t glyphCount = fonts[fnt].last - fonts[fnt].first + 1;
        size_t bitmapSize = (bitmapArraySizes != NULL && fnt < ArrSize(bitmapArraySizes))
                                ? bitmapArraySizes[fnt]
                                : 0;

        printf("  Font %zu: %u-%u (%zu glyphs), bitmap: %zu bytes\n", fnt, fonts[fnt].first,
               fonts[fnt].last, glyphCount, bitmapSize);

        /* Allocate and pack glyphs */
        GFXglyphPacked *currentGlyphs = malloc(sizeof(GFXglyphPacked) * glyphCount);
        if (!currentGlyphs) {
            printf("Error: Memory allocation failed\n");
            fclose(output);
            return 1;
        }

        for (size_t i = 0; i < glyphCount; i++) {
            currentGlyphs[i].bitmapOffset = fonts[fnt].glyph[i].bitmapOffset;
            currentGlyphs[i].height       = fonts[fnt].glyph[i].height;
            currentGlyphs[i].width        = fonts[fnt].glyph[i].width;
            currentGlyphs[i].xAdvance     = fonts[fnt].glyph[i].xAdvance;
            currentGlyphs[i].xOffset      = fonts[fnt].glyph[i].xOffset;
            currentGlyphs[i].yOffset      = fonts[fnt].glyph[i].yOffset;
        }

        /* Calculate offsets relative to current file position */
        long pos = ftell(output);

        GFXfontPacked currentFont;
        currentFont.first      = fonts[fnt].first;
        currentFont.last       = fonts[fnt].last;
        currentFont.yAdvance   = fonts[fnt].yAdvance;
        currentFont.glyph      = pos + sizeof(GFXfontPacked);
        currentFont.bitmap     = currentFont.glyph + sizeof(GFXglyphPacked) * glyphCount;
        currentFont.bitmapSize = bitmapSize;

        printf("    File offset: 0x%lx\n", pos);
        printf("    Glyph array at: 0x%u\n", currentFont.glyph);
        printf("    Bitmap data at: 0x%u\n", currentFont.bitmap);

        /* Write font metadata, glyph array, and bitmap */
        if (fwrite(&currentFont, sizeof(GFXfontPacked), 1, output) != 1) {
            printf("Error: Failed to write font metadata\n");
            free(currentGlyphs);
            fclose(output);
            return 1;
        }

        if (fwrite(currentGlyphs, sizeof(GFXglyphPacked), glyphCount, output) != glyphCount) {
            printf("Error: Failed to write glyph array\n");
            free(currentGlyphs);
            fclose(output);
            return 1;
        }

        if (bitmapSize > 0 && fonts[fnt].bitmap != NULL) {
            if (fwrite(fonts[fnt].bitmap, sizeof(uint8_t), bitmapSize, output) != bitmapSize) {
                printf("Error: Failed to write bitmap data\n");
                free(currentGlyphs);
                fclose(output);
                return 1;
            }
        }

        free(currentGlyphs);
    }

    long finalSize = ftell(output);
    fclose(output);

    printf("\nSuccess! Written to %s (%ld bytes)\n", outputFile, finalSize);
    return 0;
}
