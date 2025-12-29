#include "ImageViewer.h"
#include <GlobalVariables.h>
#include <UI/UIElements.h>

void redrawStatus(float zoom) {
    Viewport vp = tft.getViewport();
    tft.resetViewport();
    int zoompercentage = zoom * 100;
    res.DrawImage(R_LIST_MENU_BACKGROUND);
    res.DrawImage(R_LIST_HEADER_BACKGROUND);
    res.DrawImage(R_LIST_HEADER_ICONS, LM_SETTINGS);
    tft.setTextSize(1);
    changeFont(1);

    tft.setTextColor(0xFFFF);
    tft.setCursor(28, 45);
    tft.print("Image Viewer");

    if (zoompercentage != 100) {
        changeFont(0);
        tft.setCursor(210, 41);
        tft.printf("%d%%", zoompercentage);
    }
    tft.setViewport(vp);
}

void ImageViewer(const NString path) {
    NFile *activeFile = VFS.open(path);
    if (!activeFile) {
        InfoWindow("Failed to open file.");
        return;
    }
    image_data activeImage = displayPNG(path);
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
    realH = imageSrcW * zoom;
    redrawStatus(zoom);
    tft.setViewport(vp);
    activeImage = displayPNG(path, realW, realH, false);
    while (true) {
        if (zoom > 0.90f && zoom < 1.10f) { zoom = 1.0f; }
        if (!fullscreen) { redrawStatus(zoom); }
        int imageX = (visibleW / 2) - ((imageSrcW * zoom) / 2) + offsetX;
        int imageY = (fullscreen ? visibleHfs : visibleH / 2) - ((imageSrcH * zoom) / 2) + offsetY;
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
            activeFile->close();
            return;
        }
    }
    free(activeImage.buffer);
    tft.resetViewport();
    activeFile->close();
}