#include "PNGDraw.h"

void *pngOpen(const char *filename, int32_t *size) {
    File *file = new File(SD.open(filename, FILE_READ));
    *size      = file->size();
    return file;
}

void pngClose(void *handle) {
    File *file = (File *)handle;
    if (file) {
        file->close();
        delete file;
    }
}

int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length) {
    File *file = (File *)page->fHandle;
    return file->read(buffer, length);
}

int32_t pngSeek(PNGFILE *page, int32_t position) {
    File *file = (File *)page->fHandle;
    return file->seek(position);
}

bool iswallpaper = false;
void pngDraw(PNGDRAW *pDraw) {
    uint16_t lineBuffer[240];
    png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
//TODO WALLPAPER
    if (iswallpaper) {
        if (pDraw->y < 294) {
           // memcpy(wallpaper + (pDraw->y * 480), (uint8_t *)lineBuffer, 480);
        }
    } else {
       // tft.pushImage(0, pDraw->y + 26, pDraw->iWidth, 1, lineBuffer);
    }
}

// ## Draw PNG image
// This function draws a PNG image from the SD card to the screen
// @param filename: Path to the PNG file on the SD card
// @param _wallpaper: Boolean indicating if the image is a wallpaper
void drawPNG(const char *filename, bool _wallpaper) {
    

    File file = SD.open(filename, FILE_READ);
    if (!file) {
        Serial.println("Failed to open file");
        return;
    }

    int rc = png.open(filename, pngOpen, pngClose, pngRead, pngSeek, pngDraw);
    if (rc == PNG_SUCCESS) {
        //TODO
        Serial.printf("Image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());

        rc = png.decode(NULL, 0);
        png.close();
    } else {
        Serial.printf("PNG open failed: %d\n", rc);
    }

    file.close();
    
}
