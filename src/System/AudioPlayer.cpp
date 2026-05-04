#include "AudioPlayer.h"
#include <GlobalVariables.h>
#include <Input/Input.h>
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
    while (buttonsHelding() != BACK) {
        uint64_t time_s  = player->getTimeMs() / 1000;
        uint8_t  seconds = time_s % 60;
        uint8_t  minutes = time_s / 60 % 60;
        uint8_t  hours   = time_s / 3600;
        do {

            tft.setCursor(30, 200);
            tft.fillRect(0, 26, 240, 294, TFT_BLACK);
            tft.printf("%s\n       %02u:%02u:%02u", path.c_str(), hours, minutes, seconds);
            last_seconds = seconds;
            currentRenderTarget->present();
        }
        while (seconds != last_seconds);
    }
    player->stop();
    delete player;
}