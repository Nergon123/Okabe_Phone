#include "op_keypad.h"
#include "op_peripherals.h"
namespace okabe {
namespace {
constexpr uint8_t IODIRA = 0x00, IODIRB = 0x01;
constexpr uint8_t GPIOA = 0x12, GPIOB = 0x13;
constexpr uint8_t OLATA = 0x14, OLATB = 0x15;
esp_err_t write(uint8_t address, uint8_t reg, uint8_t value) {
    return writeRegister(address, reg, &value, 1);
}
}
esp_err_t initKeypad(uint8_t address) {
    // An ESP reset doesn't reset the externally powered expander. At BANK=1,
    // 0x05 is IOCON; at BANK=0 it is GPINTENB, which we also want cleared.
    esp_err_t err = write(address, 0x05, 0);
    if (err != ESP_OK) return err;
    const uint8_t setup[][2] = {
        {0x0a, 0x20}, // BANK=0, byte addressing, active-low interrupts
        {IODIRA, 0xff}, {IODIRB, 0xff}, // release all matrix lines first
        {0x02, 0}, {0x03, 0}, // no input polarity inversion
        {0x04, 0}, {0x05, 0}, // polling, no interrupts
        {OLATA, 0}, {OLATB, 0}, // outputs will drive low when selected
        {0x0c, 0xff}, {0x0d, 0xff}, // pull-ups for both input ports
    };
    for (const auto& setting : setup) {
        if ((err = write(address, setting[0], setting[1])) != ESP_OK) return err;
    }
    // Detect a bus/chip/configuration failure instead of reporting a false success.
    for (const auto& setting : setup) {
        uint8_t actual = 0;
        err = readRegister(address, setting[0], &actual, 1);
        if (err != ESP_OK) return err;
        if (actual != setting[1]) return ESP_ERR_INVALID_RESPONSE;
    }
    return ESP_OK;
}
    esp_err_t scanKeypad(uint8_t address, int& key) {
        key = 0;
        uint8_t a = 0xff, b = 0xff;
        esp_err_t err;
        // Release the previous drive port before driving the opposite port.
        if ((err = write(address, IODIRA, 0xff)) == ESP_OK &&
            (err = write(address, OLATB, 0)) == ESP_OK &&
            (err = write(address, IODIRB, 0)) == ESP_OK &&
            (err = readRegister(address, GPIOA, &a, 1)) == ESP_OK &&
            (err = write(address, IODIRB, 0xff)) == ESP_OK &&
            (err = write(address, OLATA, 0)) == ESP_OK &&
            (err = write(address, IODIRA, 0)) == ESP_OK) {
            err = readRegister(address, GPIOB, &b, 1);
        }
        esp_err_t releaseA = write(address, IODIRA, 0xff);
        esp_err_t releaseB = write(address, IODIRB, 0xff);
        if (err != ESP_OK) return err;
        if (releaseA != ESP_OK) return releaseA;
        if (releaseB != ESP_OK) return releaseB;
        uint8_t rows = static_cast<uint8_t>(~a) >> 1;
        uint8_t cols = static_cast<uint8_t>(~b) & 0x07;
        if (!rows || !cols) return ESP_OK;
        int row = __builtin_ctz(unsigned(rows)) + 1;
        int col = __builtin_ctz(unsigned(cols)) + 1;
        key = 21 - row * 3 + col;
        return ESP_OK;
    }
}
