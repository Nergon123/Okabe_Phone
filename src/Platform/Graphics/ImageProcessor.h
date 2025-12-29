#pragma once
#include <GlobalVariables.h>
struct image_data {
int srcwidth;
int srcheight;
const char* errorReason;
uint16_t *buffer;
};
uint16_t* convertRGBToRGB565(uint8_t* img, int w, int h);
uint8_t* resizePNG(uint8_t* img, int width, int height, int targetW, int targetH);
image_data displayPNG(const NString path, int w = -1, int h = -1,bool onlyParams = true);
