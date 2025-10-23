#include "FontManagement.h"

void chfont(uint8_t f) {

}
void changeFont(int ch,bool buffer) {
    currentFont = ch;

    chfont(ch);
}

// ## Write custom font
//  This function writes a custom font to the screen.
//  @param x: x position on the screen
//  @param y: y position on the screen
//  @param input: string to be written
//  @param type: type of font (0: white outcoming call font , 1: black incoming call font)
void writeCustomFont(int x, int y, NString input, int type) {
    // TODO
}