#include "FontManagement.h"

void chfont(const GFXfont *f, bool is_screen_buffer, TFT_eSprite &sbuffer) {
    if (is_screen_buffer) { sbuffer.setFreeFont(f); }
    else { tft.setFreeFont(f); }
}
void chfont(uint8_t f, bool is_screen_buffer, TFT_eSprite &sbuffer) {
    if (is_screen_buffer) { sbuffer.setTextFont(f); }
    else { tft.setTextFont(f); }
}
void changeFont(int ch, bool is_screen_buffer, TFT_eSprite &sbuffer) {
    currentFont               = ch;
    const GFXfont *GFXFonts[] = {&FONT1, &FONT2, &FONT3, &FONT4};

    if (ch == 0) { chfont(FONT0, is_screen_buffer, sbuffer); }
    else if (ch > 0 && ch <= ArraySize(GFXFonts)) {
        chfont(GFXFonts[ch - 1], is_screen_buffer, sbuffer);
    }
    else { chfont(FONT0, is_screen_buffer, sbuffer); }
}

// ## Write custom font
//  This function writes a custom font to the screen.
//  @param x: x position on the screen
//  @param y: y position on the screen
//  @param input: string to be written
//  @param type: type of font (0: white outcoming call font , 1: black incoming call font)
void writeCustomFont(int x, int y, String input, int type) {
    // TODO
}