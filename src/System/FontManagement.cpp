#include "FontManagement.h"

static fontFile_t   FONT1[]        = {{&FreeSans9pt7b, false, ""},
                                      {&FreeSans9pt8bCYR, false, ""},
                                      {nullptr, true, "/spiffs/JPfont.nfnt"}};
static fontFile_t   FONT2[]        = {{&FreeSansBold9pt7b, false, ""},
                                      {&FreeSansBold9pt8bCYR, false, ""},
                                      {nullptr, true, "/spiffs/JPfont.nfnt"}};
static fontFile_t   FONT3[]        = {{&FreeMono9pt7b, false, ""},
                                      {&FreeMono9pt8bCYR, false, ""},
                                      {nullptr, true, "/spiffs/JPfont.nfnt"}};
static fontFile_t   FONT4[]        = {{&FreeSans12pt7b, false, ""},
                                      {&FreeSans12pt8bCYR, false, ""},
                                      {nullptr, true, "/spiffs/JPfont.nfnt"}};
static fontFile_t*  fonts[]        = {FONT1, FONT2, FONT3, FONT4};
static const size_t fonts_counts[] = {
    sizeof(FONT1) / sizeof(FONT1[0]), sizeof(FONT2) / sizeof(FONT2[0]),
    sizeof(FONT3) / sizeof(FONT3[0]), sizeof(FONT4) / sizeof(FONT4[0])};

void changeFont(int ch) {
    if (ch > 0 && (size_t)ch < ArraySize(fonts)) {
        ch -= 1;
        tft.currentFont.isGFX          = true;
        tft.currentFont.isGFXFontSet   = true;
        tft.currentFont.font_file      = fonts[ch][0];
        tft.currentFont.font_set       = fonts[ch];
        tft.currentFont.font_set_count = fonts_counts[ch];
    }
    else {
        tft.currentFont.isGFX            = false;
        tft.currentFont.font_file.gfont  = nullptr;
        tft.currentFont.font_file.isFile = false;
        tft.currentFont.font_file.path   = "";
        tft.currentFont.font_set         = nullptr;
        tft.currentFont.font_set_count   = 0;
    }
}

// ## Write custom font
//
//  This function writes string with a custom font defined in resource file on the screen.
//  @param x: x position on the screen
//  @param y: y position on the screen
//  @param input: string to be written
//  @param type: type of font (0: white outcoming call font , 1: black incoming call font)
//  @note Expected char sequence: `0123456789#*X/:`
void writeCustomFont(int x, int y, NString input, int type, bool background, uint16_t bgColor) {
    NString seq     = "0123456789#*X/:";
    int     spacing = 0;
    int     image;
    switch (type) {
    case 0: image = R_OUTGOING_CALL_FONT; break;
    case 1: image = R_INCOMING_CALL_FONT; break;
    default: return; // wrong/missing type...
    }

    int xAdvance = res.GetImageDataByID(image).width;
    if (background) { tft.fillRect(x, y, (xAdvance+spacing) * input.length(), res.GetImageDataByID(image).height, bgColor); }
    for (size_t i = 0; i < input.length(); i++) {
        for (size_t u = 0; u < seq.length(); u++) {
            if (input[i] == seq[u]) {

                res.DrawImage(image, u, {x, y});
                x += xAdvance + spacing;
                break;
            }
        }
    }
}