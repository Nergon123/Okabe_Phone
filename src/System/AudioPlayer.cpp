#include "AudioPlayer.h"
#include <GlobalVariables.h>
#include <Input/Input.h>
#include <Platform/Graphics/RGB565BufferRenderTarget.h>
void printScrollingText(int x, int y, int w, NString text, int offsetX,
                        RGB565BufferRenderTarget *rgb = nullptr) {
    int           textWidth = tft.textWidth(text.c_str());
    RenderTarget *rt        = tft.activeRenderTarget;
    if (rgb) { tft.setRenderTarget(rgb); }
    if (textWidth <= w) {
        tft.setCursor(x, tft.fontHeight());
        tft.print(text.c_str());
        if (rgb) { tft.setRenderTarget(rt); }
        return;
    }
    ESP_LOGI("RERE", "x %d offsetX %d, realX %d", x, offsetX, x - offsetX);
    tft.setCursor(x - offsetX, tft.fontHeight());
    tft.print(text.c_str());
    if (rgb) { tft.setRenderTarget(rt); }
}

void AudioPlayer(NString path) {
    if (!VFS.exists(path)) {
        ESP_LOGE("AudioPlayer", "File does not exist: %s", path.c_str());
        return;
    }
    bool wasUsingBuffer = currentRenderTarget->getUseBuffer();
    currentRenderTarget->setUseBuffer(false);
    MP3Player *player = new MP3Player(audioSource);
    audioSource->init();
    player->init(path);
    player->play();
    int8_t last_seconds = INT8_MAX;
    changeFont(1);
    tft.fillRect(0, 26, 240, 294, TFT_BLACK);
    tft.setTextWrap(false);
    NString filename = path.substring(path.lastIndexOf('/') + 1, path.lastIndexOf('.'));
    res.DrawImage(R_AUDIOPLAYER_DISC);
    int16_t animFrame = 0;
    tft.setCursor(65, 93);
    tft.setTextColor(TFT_WHITE);
    res.DrawImage(R_AUDIOPLAYER_LABEL);
    ulong                     lastMillis    = hw->millis();
    int                       animDelay     = 70;
    bool                      textStopDelay = false;
    int                       dotsCount     = res.GetImageDataByID(R_CALL_ANIM_DOTS).count;
    RGB565BufferRenderTarget *rt = new RGB565BufferRenderTarget(240, tft.fontHeight() + 3);
    while (buttonsHelding() != BACK || !player->isEOF()) {
        uint64_t time_s  = player->getTimeMs() / 1000;
        uint8_t  seconds = time_s % 60;
        uint8_t  minutes = time_s / 60 % 60;
        uint8_t  hours   = time_s / 3600;
        if (hw->millis() - lastMillis > animDelay) {

            if (animFrame++ > (tft.textWidth(filename.c_str()) + 50)) {
                animFrame     = 0;
                textStopDelay = true;
            }
            lastMillis = hw->millis();
            spinAnim(15, 60, 20, 6, dotsCount - animFrame % dotsCount, 10, false);
            if (textStopDelay && animFrame == 4000 / animDelay) {
                textStopDelay = false;
                animFrame     = 0;
            }
            if (!textStopDelay) {
                rt->fillScreen(0);
                printScrollingText(30, 170, 180, filename, animFrame, rt);
                printScrollingText(30, 170, 180, filename,
                                   animFrame - tft.textWidth(filename.c_str()) - 80, rt);
                rt->CopyBufferToRT(0, 170, currentRenderTarget);
            }
        }

        if (seconds != last_seconds) {
            tft.setCursor(30, 200);

            char timeLabel[10];
            snprintf(timeLabel, sizeof(timeLabel), "%02u:%02u:%02u", hours, minutes, seconds);
            writeCustomFont(35, 220, timeLabel, 0, true);
            last_seconds = seconds;
        }
    }
    audioSource->stop();
    player->stop();
    delete player;
    delete rt;
    currentRenderTarget->setUseBuffer(wasUsingBuffer);
}