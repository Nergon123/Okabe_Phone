#ifdef HAS_I2S
#include "I2SAudio.h"
#include <Defines.h>
size_t BUFFER_SIZE = 1024;
void   I2SAudio::init() {}
bool   i2sInstalled = false;
void   I2SAudio::play(AudioStream* stream) {
    if (!stream) { return; }

    // Kill previous playback/driver first.
    stop();

    i2s_config_t cfg = {
        .mode        = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = stream->sampleRate,

        // Keep PCM5102 transport fixed at 16-bit stereo.
        .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,

        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count    = 8,
        .dma_buf_len      = 256,

        .use_apll           = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk         = 0,

        // VERY IMPORTANT on ESP32-S3.
        .bits_per_chan = I2S_BITS_PER_CHAN_16BIT,
    };

    esp_err_t err = i2s_driver_install(port, &cfg, 0, nullptr);

    if (err != ESP_OK) {
        ESP_LOGE("I2SAudio", "i2s_driver_install failed: %s", esp_err_to_name(err));
        return;
    }

    i2sInstalled = true;

    i2s_pin_config_t pins = {
        .bck_io_num   = I2S_BCK,
        .ws_io_num    = I2S_LRCK,
        .data_out_num = I2S_DIN,
        .data_in_num  = I2S_PIN_NO_CHANGE,
    };

    err = i2s_set_pin(port, &pins);

    if (err != ESP_OK) {
        ESP_LOGE("I2SAudio", "i2s_set_pin failed: %s", esp_err_to_name(err));

        i2s_driver_uninstall(port);
        i2sInstalled = false;
        return;
    }

    ESP_LOGI("I2SAudio", "Starting I2S: %lu Hz, stream channels=%u",
             (unsigned long)stream->sampleRate, (unsigned)stream->channels);

    current        = stream;
    current->state = AUDIO_PLAYING;

    BaseType_t result = xTaskCreate(taskEntry, "AudioTask", 32768, this, 5, &taskHandle);

    if (result != pdPASS) {
        ESP_LOGE("I2SAudio", "Failed to create AudioTask");

        current->state = AUDIO_STOPPED;
        current        = nullptr;
        taskHandle     = nullptr;

        i2s_zero_dma_buffer(port);
        i2s_driver_uninstall(port);
        i2sInstalled = false;
    }
}

void I2SAudio::taskEntry(void* arg) {
    I2SAudio* self = static_cast<I2SAudio*>(arg);

    self->taskLoop();

    // taskLoop owns this task from here.
    self->taskHandle = nullptr;

    vTaskDelete(nullptr);
}

void* aud_buff = nullptr;
void I2SAudio::taskLoop()
{
    if (!aud_buff) {
        aud_buff = heap_caps_malloc(
            BUFFER_SIZE,
            MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA
        );
    }

    if (!aud_buff) {
        ESP_LOGE("I2SAudio", "Failed to allocate audio buffer");

        if (current)
            current->state = AUDIO_EOF;

        current = nullptr;
        return;
    }

    while (current &&
           current->state == AUDIO_PLAYING)
    {
        size_t got = current->callback(
            current->user,
            aud_buff,
            BUFFER_SIZE
        );

        if (!got)
            break;

        size_t written = 0;

        esp_err_t err = i2s_write(
            port,
            aud_buff,
            got,
            &written,
            portMAX_DELAY
        );

        if (err != ESP_OK) {
            ESP_LOGE(
                "I2SAudio",
                "i2s_write failed: %s",
                esp_err_to_name(err)
            );
            break;
        }

        if (written != got) {
            ESP_LOGW(
                "I2SAudio",
                "short write: %u/%u",
                (unsigned)written,
                (unsigned)got
            );
        }
    }

    if (current &&
        current->state == AUDIO_PLAYING)
        current->state = AUDIO_EOF;

    current = nullptr;
}
void I2SAudio::stop() {
    if (current) { current->state = AUDIO_STOPPED; }

    /*
     * i2s_write() can be blocked, so deleting the task first ensures
     * nobody is touching the driver when we uninstall it.
     */
    if (taskHandle) {
        vTaskDelete(taskHandle);
        taskHandle = nullptr;
    }

    current = nullptr;

    if (i2sInstalled) {
        i2s_zero_dma_buffer(port);

        esp_err_t err = i2s_driver_uninstall(port);

        if (err != ESP_OK) {
            ESP_LOGW("I2SAudio", "i2s_driver_uninstall: %s", esp_err_to_name(err));
        }

        i2sInstalled = false;
    }
}
void I2SAudio::setVolume(float v) {
    if (v < 0) { v = 0; }

    if (v > 1) { v = 1; }

    volume = v;
}
#endif