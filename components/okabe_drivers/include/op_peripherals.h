#pragma once
#include <stddef.h>
#include <stdint.h>
#include <esp_err.h>
namespace okabe {
esp_err_t initI2C();
esp_err_t readRegister(uint8_t address, uint8_t reg, uint8_t* data, size_t size);
esp_err_t writeRegister(uint8_t address, uint8_t reg, const uint8_t* data, size_t size);
bool probeI2C(uint8_t address);
esp_err_t initModem();
int readModem(void* buffer, size_t size, unsigned timeoutMs);
int writeModem(const char* buffer, size_t size);
esp_err_t initBacklight();
void setBacklight(unsigned percent);
}
