#include "FontManagement.h"
static const GFXfont*  FONT1[]        = {&FreeSans9pt7b, &FreeSans9pt8bCYR,&JP_P1,&JP_P2,&JP_P3};
static const GFXfont*  FONT2[]        = {&FreeSansBold9pt7b, &FreeSansBold9pt8bCYR,&JP_P1,&JP_P2,&JP_P3};
static const GFXfont*  FONT3[]        = {&FreeMono9pt7b, &FreeMono9pt8bCYR,&JP_P1,&JP_P2,&JP_P3};
static const GFXfont*  FONT4[]        = {&FreeSans12pt7b, &FreeSans12pt8bCYR,&JP_P1,&JP_P2,&JP_P3};
static const GFXfont** fonts[]        = {FONT1, FONT2, FONT3, FONT4};
static const size_t    fonts_counts[] = {
    sizeof(FONT1) / sizeof(FONT1[0]), sizeof(FONT2) / sizeof(FONT2[0]),
    sizeof(FONT3) / sizeof(FONT3[0]), sizeof(FONT4) / sizeof(FONT4[0])};

void changeFont(int ch) {
    // 5x7 font is index 0
    if (ch == 0) {
        tft.currentFont.isGFX = false;
        tft.currentFont.font  = nullptr;
    }
    else {

        ch -= 1; // adjust to 0-based index
        if (ch >= 0 && (size_t)ch < ArraySize(fonts)) {
            tft.currentFont.isGFX          = true;
            tft.currentFont.isGFXFontSet   = true;
            tft.currentFont.font           = fonts[ch][0];
            tft.currentFont.font_set       = fonts[ch];
            tft.currentFont.font_set_count = fonts_counts[ch];
        }
        else {
            // fallback if invalid index
            tft.currentFont.isGFX = false;
            tft.currentFont.font  = nullptr;
        }
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
void writeCustomFont(int x, int y, NString input, int type) {
    NString seq     = "0123456789#*X/:";
    int     spacing = 0;
    int     image;
    switch (type) {
    case 0: image = R_OUTGOING_CALL_FONT; break;
    case 1: image = R_INCOMING_CALL_FONT; break;
    default: return; // wrong/missing type...
    }

    int xAdvance = res.GetImageDataByID(image).width;
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