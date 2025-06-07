#include "FontManagement.h"


void chfont(const GFXfont *f, bool is_screen_buffer, TFT_eSprite &sbuffer) {
    if (is_screen_buffer)
        sbuffer.setFreeFont(f);
    else
        tft.setFreeFont(f);
}
void chfont(uint8_t f, bool is_screen_buffer, TFT_eSprite &sbuffer) {
    if (is_screen_buffer)
        sbuffer.setTextFont(f);
    else
        tft.setTextFont(f);
}
void changeFont(int ch, bool is_screen_buffer, TFT_eSprite &sbuffer) {
    currentFont = ch;
    switch (ch) {
    case 0:
        chfont(1, is_screen_buffer, sbuffer);
        break;
    case 1:
        chfont(&FreeSans9pt7b, is_screen_buffer, sbuffer);
        break;
    case 2:
        chfont(&FreeSansBold9pt7b, is_screen_buffer, sbuffer);
        break;
    case 3:
        chfont(&FreeMono9pt7b, is_screen_buffer, sbuffer);
        break;
    case 4:
        chfont(&FreeSans12pt7b, is_screen_buffer, sbuffer);
        break;
    }
}

// ## Write custom font
//  This function writes a custom font to the screen.
//  @param x: x position on the screen
//  @param y: y position on the screen
//  @param input: string to be written
//  @param type: type of font (0: white outcoming call font , 1: black incoming call font)
void writeCustomFont(int x, int y, String input, int type) {
    int      w      = 21;
    int      h      = 27;
    bool     transp = false;
    uint16_t tc     = 0xFFFF;
    uint32_t addr   = 0x658ED3;
    switch (type) {
    case 0:
        break;
    case 1:
        w      = 15;
        h      = 19;
        addr   = 0x65FBC7;
        transp = true;
        break;
    default:
        break;
    }
    int size = w * h * 2;

    char buf[64];
    input.toCharArray(buf, 64);
    for (int i = 1; i < input.length() + 1; i++) {
        Serial.println(String(buf[i - 1]) + " " + String(w * (i - 1) + x));
        switch (buf[i - 1]) {
        case '#':
            drawImage(addr + (size * 10), w * (i - 1) + x, y, w, h, transp, tc);
            break;
        case '*':
            drawImage(addr + (size * 11), w * (i - 1) + x, y, w, h, transp, tc);
            break;
        case 'X':
            drawImage(addr + (size * 12), w * (i - 1) + x, y, w, h, transp, tc);
            break;
        case '/':
            drawImage(addr + (size * 13), w * (i - 1) + x, y, w, h, transp, tc);
            break;
        case ':':
            drawImage(addr + (size * 14), w * (i - 1) + x, y, w, h, transp, tc);
            break;
        default:
            if (buf[i - 1] > 47 && buf[i - 1] < 58)
                drawImage(addr + (size * (buf[i - 1] - 48)), w * (i - 1) + x, y, w, h, transp, tc);
            break;
        }
    }
}