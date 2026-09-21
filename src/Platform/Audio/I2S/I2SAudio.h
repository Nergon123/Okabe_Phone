#pragma once

#include "../Audio.h"
#include "driver/i2s_std.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <atomic>
class I2SAudio : public AudioSource {
public:
    ~I2SAudio() override;
    void init() override;
    void play(AudioStream* stream) override;
    void stop() override;
    void setVolume(float volume) override;

private:
    static void taskEntry(void* arg);
    void taskLoop();

    TaskHandle_t taskHandle = nullptr;

    AudioStream* current = nullptr;

    std::atomic<uint32_t> volumeGain{32768};

    i2s_chan_handle_t txChannel = nullptr;
    bool channelEnabled = false;
    SemaphoreHandle_t taskDone = nullptr;
    std::atomic<bool> stopRequested{false};
};