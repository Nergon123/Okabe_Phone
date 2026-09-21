#pragma once
#include <esp_err.h>
#include <stdint.h>
namespace okabe {
esp_err_t initKeypad(uint8_t address);
// A1..A7 are rows, B0..B2 are columns. Zero means no pressed key.
esp_err_t scanKeypad(uint8_t address, int& key);
}
