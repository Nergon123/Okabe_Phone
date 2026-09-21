#include "op_board.h"
#include "op_peripherals.h"
#include <driver/i2c.h>
#include <driver/uart.h>
#include <driver/ledc.h>
#include <freertos/FreeRTOS.h>
#include <vector>
namespace okabe {
static bool usablePin(int pin) {
    if (!GPIO_IS_VALID_GPIO(pin)) return false;
#ifdef CONFIG_IDF_TARGET_ESP32S3
    // SPI0/1 belongs to flash; octal PSRAM also owns GPIO33..37.
    if (pin >= 26 && pin <= 32) return false;
#ifdef CONFIG_SPIRAM_MODE_OCT
    if (pin >= 33 && pin <= 37) return false;
#endif
#endif
    return true;
}
static bool i2cReady = false, modemReady = false, backlightReady = false;
esp_err_t initI2C() {
    if (i2cReady) return ESP_OK;
    if (I2C_SDA < 0 || I2C_SCL < 0) return ESP_ERR_NOT_SUPPORTED;
    if (!usablePin(I2C_SDA) || !usablePin(I2C_SCL)) return ESP_ERR_INVALID_ARG;
    i2c_config_t config = {};
    config.mode = I2C_MODE_MASTER;
    config.sda_io_num = I2C_SDA;
    config.scl_io_num = I2C_SCL;
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    config.master.clk_speed = 100000;
    esp_err_t err = i2c_param_config(I2C_NUM_0, &config);
    if (err == ESP_OK) err = i2c_driver_install(I2C_NUM_0, config.mode, 0, 0, 0);
    i2cReady = err == ESP_OK;
    return err;
}
esp_err_t readRegister(uint8_t address, uint8_t reg, uint8_t* data, size_t size) {
    if (!i2cReady) return ESP_ERR_INVALID_STATE;
    return i2c_master_write_read_device(I2C_NUM_0, address, &reg, 1, data, size, pdMS_TO_TICKS(100));
}
esp_err_t writeRegister(uint8_t address, uint8_t reg, const uint8_t* data, size_t size) {
    if (!i2cReady) return ESP_ERR_INVALID_STATE;
    std::vector<uint8_t> bytes(size + 1);
    bytes[0] = reg;
    for (size_t i = 0; i < size; ++i) bytes[i + 1] = data[i];
    return i2c_master_write_to_device(I2C_NUM_0, address, bytes.data(), bytes.size(), pdMS_TO_TICKS(100));
}
bool probeI2C(uint8_t address) {
    if (!i2cReady) return false;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) return false;
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, address << 1, true);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return err == ESP_OK;
}
esp_err_t initModem() {
    if (modemReady) return ESP_OK;
    if (!OKABE_MODEM_ENABLED) return ESP_ERR_NOT_SUPPORTED;
    if (SIM_RX_PIN < 0 || SIM_TX_PIN < 0) return ESP_ERR_NOT_SUPPORTED;
    if (!usablePin(SIM_RX_PIN) || !usablePin(SIM_TX_PIN)) return ESP_ERR_INVALID_ARG;
    uart_config_t cfg = {};
    cfg.baud_rate = 115200;
    cfg.data_bits = UART_DATA_8_BITS;
    cfg.parity = UART_PARITY_DISABLE;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    cfg.source_clk = UART_SCLK_DEFAULT;
    esp_err_t err = uart_param_config(UART_NUM_1, &cfg);
    if (err == ESP_OK) err = uart_set_pin(UART_NUM_1, SIM_TX_PIN, SIM_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err == ESP_OK) err = uart_driver_install(UART_NUM_1, 2048, 0, 0, nullptr, 0);
    modemReady = err == ESP_OK;
    return err;
}
int readModem(void* buffer, size_t size, unsigned timeoutMs) {
    return modemReady ? uart_read_bytes(UART_NUM_1, buffer, size, pdMS_TO_TICKS(timeoutMs)) : -1;
}
int writeModem(const char* buffer, size_t size) {
    return modemReady ? uart_write_bytes(UART_NUM_1, buffer, size) : -1;
}
esp_err_t initBacklight() {
    if (TFT_BL < 0) return ESP_OK;
    ledc_timer_config_t timer = {};
    timer.speed_mode = LEDC_LOW_SPEED_MODE;
    timer.duty_resolution = LEDC_TIMER_8_BIT;
    timer.timer_num = LEDC_TIMER_0;
    timer.freq_hz = 5000;
    timer.clk_cfg = LEDC_AUTO_CLK;
    esp_err_t err = ledc_timer_config(&timer);
    ledc_channel_config_t channel = {};
    channel.gpio_num = TFT_BL;
    channel.speed_mode = LEDC_LOW_SPEED_MODE;
    channel.channel = LEDC_CHANNEL_0;
    channel.timer_sel = LEDC_TIMER_0;
    if (err == ESP_OK) err = ledc_channel_config(&channel);
    backlightReady = err == ESP_OK;
    return err;
}
void setBacklight(unsigned percent) {
    if (!backlightReady) return;
    if (percent > 100) percent = 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, percent * 255 / 100);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}
}
