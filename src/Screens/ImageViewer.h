#include <Platform/NString.h>
#pragma once
enum ImageMode {
    IMG_CENTERED = 0,
    IMG_TILED,
    IMG_FILLED,
    IMG_STRETCHED,
    IMG_FIT_HORIZONTALY,
    IMG_FIT_VERTICALY
};
void ImageViewer(const NString path);
void drawImageWithMode(NString path, ImageMode mode, int x = 0, int y = 0, int w = 240,
                       int h = 294, bool * fail = nullptr);