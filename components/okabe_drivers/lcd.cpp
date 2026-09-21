#include "op_board.h"
#include "op_lcd.h"
#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <esp_lcd_panel_io.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cstring>
#include <algorithm>
namespace okabe {
static esp_lcd_panel_io_handle_t io = nullptr;
// A small internal DMA staging block also supports framebuffers in PSRAM.
static constexpr int dmaPixels = 240 * 16;
static uint16_t* dmaBlock = nullptr;
static esp_err_t command(int cmd, const void* data = nullptr, size_t size = 0) {
    return esp_lcd_panel_io_tx_param(io, cmd, data, size);
}
esp_err_t initLCD() {
    if (io) return ESP_OK;
    spi_bus_config_t bus = {};
    bus.mosi_io_num = TFT_MOSI;
    bus.miso_io_num = TFT_MISO;
    bus.sclk_io_num = TFT_SCLK;
    bus.quadwp_io_num = -1;
    bus.quadhd_io_num = -1;
    bus.max_transfer_sz = dmaPixels * sizeof(uint16_t);
    esp_err_t err = spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO);
    if (err != ESP_OK) return err;
    esp_lcd_panel_io_spi_config_t config = {};
    config.cs_gpio_num = TFT_CS;
    config.dc_gpio_num = TFT_DC;
    config.spi_mode = 0;
    config.pclk_hz = SPI_FREQUENCY;
    config.trans_queue_depth = 1;
    config.lcd_cmd_bits = 8;
    config.lcd_param_bits = 8;
    err = esp_lcd_new_panel_io_spi(SPI2_HOST, &config, &io);
    if (err != ESP_OK) return err;
    dmaBlock = static_cast<uint16_t*>(heap_caps_malloc(dmaPixels * sizeof(uint16_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    if (!dmaBlock) return ESP_ERR_NO_MEM;
    if (TFT_RST >= 0) {
        gpio_reset_pin(static_cast<gpio_num_t>(TFT_RST));
        gpio_set_direction(static_cast<gpio_num_t>(TFT_RST), GPIO_MODE_OUTPUT);
        gpio_set_level(static_cast<gpio_num_t>(TFT_RST), 0);
        vTaskDelay(pdMS_TO_TICKS(20));
        gpio_set_level(static_cast<gpio_num_t>(TFT_RST), 1);
        vTaskDelay(pdMS_TO_TICKS(120));
    }
    if ((err = command(0x01)) != ESP_OK) return err;
    vTaskDelay(pdMS_TO_TICKS(150));
#ifndef OKABE_S3
    // ILI9341 power / VCOM settings for the prototype panel.
    const uint8_t power1[] = {0x23}, power2[] = {0x10}, vcom[] = {0x3e, 0x28};
    if ((err = command(0xc0, power1, sizeof(power1))) != ESP_OK) return err;
    if ((err = command(0xc1, power2, sizeof(power2))) != ESP_OK) return err;
    if ((err = command(0xc5, vcom, sizeof(vcom))) != ESP_OK) return err;
#endif
    uint8_t format = 0x55; // 16-bit RGB565
    if ((err = command(0x3a, &format, 1)) != ESP_OK) return err;
#ifdef OKABE_S3
    uint8_t madctl = 0x48; // ST7789 portrait: MX corrects this panel's horizontal orientation, BGR
#else
    uint8_t madctl = 0x48; // ILI9341 portrait: mirror X, BGR
#endif
    if ((err = command(0x36, &madctl, 1)) != ESP_OK) return err;
#ifdef INV_DISP
    if ((err = command(0x21)) != ESP_OK) return err;
#else
    if ((err = command(0x20)) != ESP_OK) return err;
#endif
    if ((err = command(0x11)) != ESP_OK) return err;
    vTaskDelay(pdMS_TO_TICKS(120));
    return command(0x29);
}
esp_err_t drawLCD(int x, int y, int width, int height, const uint16_t* pixels, int stride) {
    if (!io || !dmaBlock) return ESP_ERR_INVALID_STATE;
    if (stride == 0) stride = width;
    if (!pixels || width <= 0 || height <= 0 || width > 240 || height > 320 ||
        x < 0 || y < 0 || x > 240 - width || y > 320 - height || stride < width)
        return ESP_ERR_INVALID_ARG;
    const uint8_t cols[] = {uint8_t(x >> 8), uint8_t(x), uint8_t((x + width - 1) >> 8), uint8_t(x + width - 1)};
    const uint8_t rows[] = {uint8_t(y >> 8), uint8_t(y), uint8_t((y + height - 1) >> 8), uint8_t(y + height - 1)};
    esp_err_t err = command(0x2a, cols, 4);
    if (err != ESP_OK) return err;
    if ((err = command(0x2b, rows, 4)) != ESP_OK) return err;
    for (int row = 0; row < height;) {
        const int rows = std::min(height - row, dmaPixels / width);
        if (stride == width) {
            memcpy(dmaBlock, pixels + row * stride, rows * width * sizeof(uint16_t));
        } else {
            for (int r = 0; r < rows; ++r)
                memcpy(dmaBlock + r * width, pixels + (row + r) * stride, width * sizeof(uint16_t));
        }
        err = esp_lcd_panel_io_tx_color(io, row == 0 ? 0x2c : 0x3c,
                                       dmaBlock, rows * width * sizeof(uint16_t));
        if (err != ESP_OK) return err;
        // Drain once per block before reusing it, preserving the synchronous API.
        if ((err = command(-1)) != ESP_OK) return err;
        row += rows;
    }
    return ESP_OK;
}
}
