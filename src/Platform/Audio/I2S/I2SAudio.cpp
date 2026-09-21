#include <Defines.h>
#ifdef HAS_I2S
#include "I2SAudio.h"
#include "PCMVolume.h"
#include "esp_log.h"

namespace {
constexpr size_t BUFFER_SIZE = 1024;
constexpr uint32_t WRITE_TIMEOUT_MS = 100;
constexpr const char* TAG = "I2SAudio";
}

I2SAudio::~I2SAudio() {
    stop();
    if (taskDone) vSemaphoreDelete(taskDone);
}

void I2SAudio::init() {}

void I2SAudio::play(AudioStream* stream) {
    if (!stream) return;
    stop();
    stream->state = AUDIO_STOPPED;
    if (!stream->callback || !stream->sampleRate || stream->format != AUDIOFMT_S16) {
        ESP_LOGE(TAG, "Invalid stream: signed 16-bit PCM is required");
        return;
    }
    if (!taskDone) taskDone = xSemaphoreCreateBinary();
    if (!taskDone) {
        ESP_LOGE(TAG, "Failed to create audio completion semaphore");
        return;
    }

    i2s_chan_config_t channelConfig = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    channelConfig.dma_desc_num = 8;
    channelConfig.dma_frame_num = 256;
    channelConfig.auto_clear = true;
    channelConfig.intr_priority = 1;
    esp_err_t err = i2s_new_channel(&channelConfig, &txChannel, nullptr);
    if (err == ESP_OK) {
        i2s_std_config_t config = {};
        config.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(stream->sampleRate);
        // Preserve the existing PCM5102 transport: Philips I2S, 16-bit stereo.
        config.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
            I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
        config.gpio_cfg.mclk = I2S_GPIO_UNUSED;
        config.gpio_cfg.bclk = static_cast<gpio_num_t>(I2S_BCK);
        config.gpio_cfg.ws = static_cast<gpio_num_t>(I2S_LRCK);
        config.gpio_cfg.dout = static_cast<gpio_num_t>(I2S_DIN);
        config.gpio_cfg.din = I2S_GPIO_UNUSED;
        err = i2s_channel_init_std_mode(txChannel, &config);
    }
    if (err == ESP_OK) {
        err = i2s_channel_enable(txChannel);
        channelEnabled = err == ESP_OK;
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2S channel setup failed: %s", esp_err_to_name(err));
        stop();
        return;
    }

    current = stream;
    stopRequested.store(false);
    current->state = AUDIO_PLAYING;
    if (xTaskCreate(taskEntry, "AudioTask", 32768, this, 5, &taskHandle) != pdPASS) {
        taskHandle = nullptr;
        ESP_LOGE(TAG, "Failed to create AudioTask");
        stop();
        return;
    }
    ESP_LOGI(TAG, "Starting I2S: %lu Hz, 16-bit stereo", (unsigned long)stream->sampleRate);
}

void I2SAudio::taskEntry(void* arg) {
    auto* self = static_cast<I2SAudio*>(arg);
    self->taskLoop();
    // No access to self or the stream after signalling completion.
    xSemaphoreGive(self->taskDone);
    vTaskDelete(nullptr);
}

void I2SAudio::taskLoop() {
    // The channel API copies into its own DMA buffers.
    alignas(4) uint8_t buffer[BUFFER_SIZE];
    bool failed = false;
    while (!stopRequested.load() && current->state == AUDIO_PLAYING) {
        const size_t got = current->callback(current->user, buffer, sizeof(buffer));
        if (!got) break;
        if (got > sizeof(buffer) || got % 4 != 0) {
            ESP_LOGE(TAG, "Invalid stereo PCM block size: %u", (unsigned)got);
            failed = true;
            break;
        }
        // Scale once per block, so retries never attenuate samples twice.
        pcm_volume::apply(buffer, got, volumeGain.load(std::memory_order_relaxed));
        size_t offset = 0;
        while (offset < got && !stopRequested.load()) {
            size_t written = 0;
            const esp_err_t err = i2s_channel_write(
                txChannel, buffer + offset, got - offset, &written, WRITE_TIMEOUT_MS);
            offset += written;
            if (err != ESP_OK && err != ESP_ERR_TIMEOUT) {
                ESP_LOGE(TAG, "I2S write failed: %s", esp_err_to_name(err));
                failed = true;
                break;
            }
        }
        if (failed) break;
    }
    if (current->state == AUDIO_PLAYING)
        current->state = stopRequested.load() || failed ? AUDIO_STOPPED : AUDIO_EOF;
}

void I2SAudio::stop() {
    stopRequested.store(true);
    if (taskHandle) {
        // Let a pending write return before deleting the channel and its locks.
        xSemaphoreTake(taskDone, portMAX_DELAY);
        taskHandle = nullptr;
    }
    if (current) current->state = AUDIO_STOPPED;
    current = nullptr;
    if (txChannel) {
        if (channelEnabled) {
            const esp_err_t err = i2s_channel_disable(txChannel);
            if (err != ESP_OK) ESP_LOGW(TAG, "I2S disable: %s", esp_err_to_name(err));
            channelEnabled = false;
        }
        const esp_err_t err = i2s_del_channel(txChannel);
        if (err != ESP_OK) ESP_LOGW(TAG, "I2S delete: %s", esp_err_to_name(err));
        txChannel = nullptr;
    }
}

void I2SAudio::setVolume(float v) {
    volumeGain.store(pcm_volume::gain(v), std::memory_order_relaxed);
}
#endif
