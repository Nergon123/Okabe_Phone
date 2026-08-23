#pragma once

#include "../Audio.h"
#include "driver/i2s.h"
#include <esp_heap_caps.h>
class I2SAudio : public AudioSource {
public:
    void init() override;
    void play(AudioStream* stream) override;
    void stop() override;
    void setVolume(float volume) override;

private:
    static void taskEntry(void* arg);
    void taskLoop();

    TaskHandle_t taskHandle = nullptr;

    AudioStream* current = nullptr;

    float volume = 1.0f;

    i2s_port_t port = I2S_NUM_0;
};