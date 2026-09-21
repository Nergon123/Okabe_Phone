#pragma once
#include "Platform/Hardware/Hardware.h"
#ifndef PC
class DEV_ESP32 : public iHW {
public:
    void init() override;
    void initStorage() override;
    void postScreenInit() override;
    ulong micros() override;
    ulong millis() override;
    void delay(ulong ms) override;
    void timeSet(time_t t) override;
    time_t timeGet() override;
    const char* getDeviceName() override;
    void shutdown() override;
    void reboot() override;
    void setScreenBrightness(int8_t value) override;
    char getCharInput() override;
    int getWifiStrength() override;
    int getKeyInput() override;
    int getBatteryCharge() override;
    bool isCharging() override;
    RenderTarget* GetScreen() override;
    HttpAnswer httpSend(HttpMethod method, const NString& url, NString& payload,
                        const std::vector<HttpHeader>& headers, uint16_t timeout) override;
    void downloadFile(NString& url, IFile* file,
                      std::function<void(size_t, size_t)> progress = nullptr) override;
    uint32_t crc32(uint32_t crc, const uint8_t* buf, size_t len) override;
private:
    bool keypad = false, charger = false;
    uint8_t keypadAddress = 0x20;
    ulong keypadRetryAt = 0;
    void initKeypad();
};
#endif
