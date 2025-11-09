#include "Notifications.h"
#include "../System/ResourceSystem.h"
#include "Platform/Graphics/RenderTargets.h"
#include "Platform/Graphics/RGB565BufferRenderTarget.h"
// ## Draw status bar
// This function draws the status bar on the screen
// @param force: If true, force redraw of the status bar if false only redraw if time has changed
void drawStatusBar(bool force) {

    sBarChanged += force;
    time(&systemTime);
    tm sbtime = *gmtime(&systemTime);
    if (sbtime.tm_min != systemTimeInfo.tm_min) {
        // Serial.printf("\nSBTIME MIN %d , STI MIN %d\n", sbtime.tm_min, systemTimeInfo.tm_min);
        SaveTime(systemTime);
        sBarChanged = true;
    }

    if (sBarChanged) {
        int  viewport_x     = tft.getViewportX();
        int  viewport_y     = tft.getViewportY();
        int  viewport_w     = tft.getViewportWidth();
        int  viewport_h     = tft.getViewportHeight();
        tft.resetViewport();

        if (_signal < 0) { _signal = 0; }
        if (_signal > 3) { _signal = 3; }
        RenderTarget* before = currentRenderTarget;
        RenderTarget* buf = new RGB565BufferRenderTarget(240,26);
        sBarChanged = false;
        charge      = getChargeLevel();
        tft.setRenderTarget(buf);
         res.DrawImage(R_STATUSBAR_BACKGROUND);
         res.DrawImage(R_SIGNAL_STRENGTH, _signal);
         res.DrawImage(R_BATTERY_CHARGE, charge);
        //  tft.print(NString(charge) + NString("%"));
         changeFont(1);
         tft.setTextSize(1);
         tft.setTextColor(TFT_LIGHTGREY);
         tft.setCursor(102, 19);
         tft.printf("%02d:%02d", sbtime.tm_hour, sbtime.tm_min);
        if (isScreenLocked) {
             changeFont(0);
             tft.setCursor(0, 0);
             tft.setTextSize(1);
             tft.setTextColor(TFT_WHITE);
             tft.print("KEYBOARD IS LOCKED HOLD * TO UNLOCK");
        }
        tft.setRenderTarget(before);
        before->pushBuffer(0,0,buf->getWidth(),buf->getHeight(),buf->getBuffer(),0,0);
        before->present();
        tft.setViewport(viewport_x, viewport_y, viewport_w, viewport_h);
    }
}
// Function to show a confirmation window
// This function is called when the user wants to confirm an action
// @param reason The reason for the confirmation
// @return true if the user confirms, false if the user cancels
bool confirmation(NString reason,NString yes, NString no) {
    drawWallpaper();
    res.DrawImage(R_FULL_NOTIFICATION);
    changeFont(1);
    tft.setCursor(45, 110);
    tft.setTextColor(0);
    tft.println("CONFIRMATION");
    if (tft.textWidth(reason) < 240) { tft.setCursor((230 - tft.textWidth(reason)) / 2, 130); }
    else { tft.setCursor(0, 130); }
    tft.println(SplitString(reason));

    int  pos = 0;
    int  direction;
    bool exit = false;

    while (!exit) {
        if (button(yes, 120, 190, 80, 30, pos, &direction)) { return true; }
        else if (button(no, 30, 190, 80, 30, !pos, &direction)) { return false; }
        if (direction > 1) { pos = !pos; }
        if (direction == BACK) return false;
    }
    return false;
}

// Function to show an error window
// This function is called when an error occurs
// @param reason The reason for the error
void ErrorWindow(NString reason) {

    int xpos = 0;
    drawWallpaper();
    res.DrawImage(R_FULL_NOTIFICATION);
    tft.setTextSize(1);
    tft.setCursor(80, 120);
    changeFont(1);
    tft.setTextColor(TFT_RED);
    tft.println("ERROR");
    tft.setTextColor(0);
    if (tft.textWidth(reason) < 240) { xpos = (240 - tft.textWidth(reason)) / 2; }
    tft.setCursor(xpos, 150);
    tft.print(SplitString(reason));
    while (buttonsHelding() == -1);
}
