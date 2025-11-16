#include "FontManagement.h"

void changeFont(int ch,bool buffer) {
    // 5x7 font is index 0
    if (ch == 0) {
        tft.currentFont.isGFX = false;
        tft.currentFont.font  = nullptr;
    } else {
        static const GFXfont* fonts[] = { &FONT1, &FONT2, &FONT3, &FONT4 };
        ch -= 1; // adjust to 0-based index
        if (ch >= 0 && ch < ArraySize(fonts)) {
            tft.currentFont.isGFX = true;
            tft.currentFont.font  = fonts[ch];
        } else {
            // fallback if invalid index
            tft.currentFont.isGFX = false;
            tft.currentFont.font  = nullptr;
        }
    }
}

// ## Write custom font
//  This function writes a custom font to the screen.
//  @param x: x position on the screen
//  @param y: y position on the screen
//  @param input: string to be written
//  @param type: type of font (0: white outcoming call font , 1: black incoming call font)
void writeCustomFont(int x, int y, NString input, int type) {
    #warning writeCustomFont not implemented
}