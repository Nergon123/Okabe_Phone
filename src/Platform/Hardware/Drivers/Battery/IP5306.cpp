#ifndef PC
#include "IP5306.h"

/*

THIS LIBRARY WAS DOWNLOADED FROM https://github.com/rynskyi/IP5306

*/

/*
    Notice:
    endTransmission() returns:
    0: success.
    1: data too long to fit in transmit buffer.
    2: received NACK on transmit of address.
    3: received NACK on transmit of data.
    4: other error.
    5: timeout
*/

void IP5306::begin(uint8_t, uint8_t) { okabe::initI2C(); }
uint8_t IP5306::writeBytes(uint8_t addr, uint8_t reg, uint8_t* data, uint8_t quantity) {
    return okabe::writeRegister(addr, reg, data, quantity) == ESP_OK ? 0 : 1;
}
uint8_t IP5306::readBytes(uint8_t addr, uint8_t reg, uint8_t* data, uint8_t quantity) {
    return okabe::readRegister(addr, reg, data, quantity) == ESP_OK ? quantity : 0;
}

uint8_t IP5306::setup(uint8_t initPrms /*= IP5306_DEFAULT_SETUP*/) {
    return this->writeBytes(IP5306_ADDR, IP5306_REG_SYS_CTL0, &initPrms, 1);
}

uint8_t IP5306::isChargerConnected() {
    uint8_t data = 0;
    this->readBytes(IP5306_ADDR, IP5306_REG_READ0, &data, 1);
    return (data & (1 << 3)) ? 1 : 0;
}

uint8_t IP5306::isChargeFull() {
    uint8_t data = 0;
    this->readBytes(IP5306_ADDR, IP5306_REG_READ1, &data, 1);
    return (data & (1 << 3)) ? 1 : 0;
}

uint8_t IP5306::getBatteryLevel() {
    uint8_t data = 0;
    if (this->readBytes(IP5306_ADDR, IP5306_REG_UNKNOW, &data, 1) != 1) return 0;
    switch (data & 0xF0) {
    case 0xE0: return 25;
    case 0xC0: return 50;
    case 0x80: return 75;
    case 0x00: return 100;
    default: return 0;
    }
}
IP5306 bat;
#endif