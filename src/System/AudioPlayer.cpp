#include "AudioPlayer.h"
#include <GlobalVariables.h>
#include <Input/Input.h>

void printScrollingText(int x, int y, int w, NString text, int offsetX) {
    int textWidth = tft.textWidth(text.c_str());
    if (textWidth <= w) {
        tft.setCursor(x, y);
        tft.print(text.c_str());
        return;
    }
    tft.setCursor(x - offsetX, y);
    tft.print(text.c_str());
}

void AudioPlayer(NString path) {
    if (!VFS.exists(path)) {
        ESP_LOGE("AudioPlayer", "File does not exist: %s", path.c_str());
        return;
    }
    MP3Player *player = new MP3Player(audioSource);
    audioSource->init();
    player->init(path);
    player->play();
    int last_seconds = 0;
    changeFont(1);
    tft.fillRect(0, 26, 240, 294, TFT_BLACK);
    tft.setTextWrap(false);
    NString filename = path.substring(path.lastIndexOf('/') + 1, path.lastIndexOf('.'));
    res.DrawImage(R_AUDIOPLAYER_DISC);
    int16_t animFrame  = 0;
    tft.setCursor(65, 93);
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
    res.DrawImage(R_AUDIOPLAYER_LABEL);
    ulong lastMillis     = hw->millis();
    int   animDelay      = 70;
    bool  textStopDelay  = false;
    int dotsCount = res.GetImageDataByID(R_CALL_ANIM_DOTS).count;
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
            spinAnim(15, 60, 20, 6, dotsCount - animFrame % dotsCount);
            if (textStopDelay && animFrame == 4000 / animDelay) {
                textStopDelay = false;
                animFrame     = 0;
            }
            if (!textStopDelay) {
                printScrollingText(30, 170, 180, filename, animFrame);
                printScrollingText(30, 170, 180, filename,
                                   animFrame - tft.textWidth(filename.c_str()) - 80);
            }
        }
        do {

            tft.setCursor(30, 200);

            char timeLabel[10];
            snprintf(timeLabel, sizeof(timeLabel), "%02u:%02u:%02u", hours, minutes, seconds);
            writeCustomFont(35, 220, timeLabel, 0, true, TFT_BLACK);
            last_seconds = seconds;
            currentRenderTarget->present();
        }
        while (seconds != last_seconds);
    }
    audioSource->stop();
    player->stop();
    delete player;
}