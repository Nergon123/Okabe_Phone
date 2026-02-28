#include "ImageViewer.h"
#include <GlobalVariables.h>
#include <Platform/Graphics/RGB565BufferRenderTarget.h>
#include <UI/UIElements.h>
void redrawStatus(float zoom) {
    Viewport vp = tft.getViewport();
    tft.resetViewport();
    int zoompercentage = zoom * 100;
    res.DrawImage(R_LIST_MENU_BACKGROUND);
    drawHeader("Image Viewer", LM_SETTINGS, NString::format("%d%%", zoompercentage));
    tft.setViewport(vp);
}

void ImageViewer(const NString path) {
    if (path.isEmpty()) { return; }
    image_data activeImage = displayPNG(path);
    if (activeImage.srcheight <= 0 || activeImage.srcwidth <= 0) {
        InfoWindow("Failed to open file");
    }
    if (activeImage.errorReason) {
        InfoWindow(activeImage.errorReason);
        return;
    }

    int   imageSrcW = activeImage.srcwidth;
    int   imageSrcH = activeImage.srcheight;
    float zoom      = 1.0f;

    bool     fullscreen = false;
    int      visibleW   = 240;
    int      visibleH   = 269;
    int      visibleHfs = 320;
    int      offsetX    = 0;
    int      offsetY    = 0;
    bool     priorWidth = visibleW - imageSrcW < visibleH - imageSrcH;
    int      realW      = imageSrcW * zoom;
    int      realH      = imageSrcW * zoom;
    Viewport vp         = Viewport(0, 320 - visibleH, visibleW, visibleH);
    if (priorWidth) { zoom = (float)visibleW / (float)imageSrcW; }
    else { zoom = fullscreen ? (float)visibleHfs : (float)visibleH / (float)imageSrcH; }
    realW = imageSrcW * zoom;
    realH = imageSrcH * zoom;
    redrawStatus(zoom);
    tft.setViewport(vp);
    activeImage = displayPNG(path, realW, realH, false);
    while (true) {
        if (zoom > 0.90f && zoom < 1.10f) { zoom = 1.0f; }
        if (!fullscreen) { redrawStatus(zoom); }
        int imageX = (visibleW / 2) - ((realW) / 2) + offsetX;
        int imageY = (fullscreen ? visibleHfs : visibleH / 2) - ((realH) / 2) + offsetY;
        tft.pushImage(imageX, imageY, realW, realH, activeImage.buffer);
        currentRenderTarget->present();
        int button = -1;
        while (button == -1) { button = buttonsHelding(!fullscreen); };

        switch (button) {
        case UP:
            if (imageY > -realH) { offsetY -= realH / 10; }
            break;
        case DOWN:
            if (imageY < fullscreen ? visibleHfs : visibleH) { offsetY += realH / 10; }
            break;
        case LEFT:
            if (imageX > -realW) { offsetX -= realW / 10; }
            break;
        case RIGHT:
            if (imageX < visibleW) { offsetX += realW / 10; }
            break;
        case '*': // zoom-
            zoom -= 0.25f;
            zoom  = constrain(zoom, 0.05f, 5.0f);
            realW = imageSrcW * zoom;
            realH = imageSrcW * zoom;
            free(activeImage.buffer);
            activeImage = displayPNG(path, realW, realH, false);
            break;
        case SELECT:
        case '#': // zoom+
            zoom += 0.25f;
            zoom  = constrain(zoom, 0.05f, 5.0f);
            realW = imageSrcW * zoom;
            realH = imageSrcW * zoom;
            free(activeImage.buffer);

            activeImage = displayPNG(path, realW, realH, false);
            break;
        case BACK:
            free(activeImage.buffer);
            tft.resetViewport();
            return;
        }
    }
    free(activeImage.buffer);
    tft.resetViewport();
}

void drawImageWithMode(NString path, ImageMode mode, int x, int y, int w, int h, bool * fail) {
    if (fail) { *fail = false; }
    if (path.isEmpty()) { if (fail) { *fail = true; } return; }
    if (w <= 0 || h <= 0) {
        InfoWindow("Invalid image dimensions");
        return;
    }
    image_data activeImage = displayPNG(path); // metadata only
    if (activeImage.srcheight <= 0 || activeImage.srcwidth <= 0) {
        InfoWindow("Failed to open Image!");
        if (fail) { *fail = true; }
        return;
    }
    int imageW = activeImage.srcwidth;
    int imageH = activeImage.srcheight;
    tft.setViewport(x, y, w, h);
    switch (mode) {
    case IMG_CENTERED: {
        activeImage = displayPNG(path, imageW, imageH, false);
        if (activeImage.buffer) {
            tft.pushImage((w / 2) - (imageW / 2), (h / 2) - (imageH / 2), imageW, imageH,
                          activeImage.buffer);
            free(activeImage.buffer);
        }
        else { if (fail) { *fail = true; } InfoWindow("Error when opening file."); }

        tft.resetViewport();
        return;
    }

    case IMG_TILED: {

        activeImage = displayPNG(path, imageW, imageH, false);
        if (activeImage.buffer) {
            for (int iy = 0; iy < h; iy += imageH) {
                for (int ix = 0; ix < w; ix += imageW) {
                    tft.pushImage(ix, iy, imageW, imageH, activeImage.buffer);
                }
            }
            free(activeImage.buffer);
        }
        else { if (fail) { *fail = true; } InfoWindow("Error when opening file."); }

        tft.resetViewport();
        return;
    }
    case IMG_FILLED: {
        bool priorWidth = h - imageH > w - imageW;

        float zoom = priorWidth ? (float)w / (float)imageW : (float)h / (float)imageH;

        int newImageH = imageH * zoom;
        int newImageW = imageW * zoom;
        activeImage   = displayPNG(path, newImageH, newImageW, false);
        if (activeImage.buffer) {
            tft.pushImage((w - newImageW) / 2, (h - newImageH) / 2, newImageW, newImageH,
                          activeImage.buffer);

            free(activeImage.buffer);
        }
        else { if (fail) { *fail = true; } InfoWindow("Error when opening file."); }
        
        tft.resetViewport();
        return;
    }
    case IMG_STRETCHED: {
        activeImage = displayPNG(path, w, h, false);
        if (activeImage.buffer) {
            tft.pushImage(0, 0, w, h, activeImage.buffer);
            free(activeImage.buffer);
        }
        else { if (fail) { *fail = true; } InfoWindow("Error when opening file."); }
        
        tft.resetViewport();
        return;
    }
    case IMG_FIT_HORIZONTALY:
    case IMG_FIT_VERTICALY: {
        bool priorWidth = (mode == IMG_FIT_HORIZONTALY);

        float zoom = priorWidth ? (float)w / (float)imageW : (float)h / (float)imageH;

        int newImageH = imageH * zoom;
        int newImageW = imageW * zoom;
        activeImage   = displayPNG(path, newImageH, newImageW, false);
        if (activeImage.buffer) {
            tft.pushImage((w - newImageW) / 2, (h - newImageH) / 2, newImageW, newImageH,
                          activeImage.buffer);

            free(activeImage.buffer);
        }
        else { if (fail) { *fail = true; } InfoWindow("Error when opening file."); }
        
        tft.resetViewport();
        return;
    }
    default: if (fail) { *fail = true; } InfoWindow("Invalid image Mode (" + NString((int)mode) + ")"); break;
    }
    tft.resetViewport();
}