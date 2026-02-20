#pragma once
#include "stdlib.h"
#include <Platform/FileSystem/FileSystem.h>
#include <Platform/Graphics/RenderTargets.h>
#include <Platform/NString.h>
#include <functional>
#include <vector>
enum CPU_SPEED { CPU_IDLE, CPU_DEFAULT, CPU_FAST };
struct HttpAnswer {
    int     code;
    NString response;
};
struct HttpHeader {
    NString name;
    NString content;
};
enum class HttpMethod { GET, POST, PUT, DELETE_, PATCH };

class iHW {
  public:
    virtual ~iHW();
    virtual void  init() {};
    virtual void  initStorage() {};
    virtual void  postScreenInit() {};
    virtual ulong micros() { return 0; };
    virtual ulong millis() { return micros() / 1000; };
    virtual void  delay(ulong ms) {
        ulong mil = millis();
        while (millis() < mil + ms);
    }
    virtual int         getWifiStrength() { return -1; }
    virtual void        setCPUSpeed(CPU_SPEED speed) { (void)speed; };
    virtual CPU_SPEED   getCPUSpeed() { return CPU_DEFAULT; };
    virtual void        updateFrequencies() {};
    virtual const char* getDeviceName() { return "Unknown"; };
    virtual void        shutdown() {};
    virtual void        reboot() {};
    virtual uint32_t    crc32(uint32_t crc, const uint8_t* buf, size_t len) {
        (void)crc;
        (void)buf;
        (void)len;
        return 0xFFFFFFFF;
    };
    virtual void downloadFile(NString& url, IFile* fileToDownload,
                              std::function<void(size_t, size_t)> progressCallback = nullptr) {
        (void)url;
        (void)fileToDownload;
        (void)progressCallback;
    };
    virtual void       timeSet(time_t t) { (void)t; };
    virtual time_t     timeGet() { return 0; };
    virtual void       setScreenBrightness(int8_t value) { (void)value; };
    virtual char       getCharInput() { return 0; };
    virtual int        getKeyInput() { return 0; };
    virtual int        getBatteryCharge() { return 3; };
    virtual bool       isCharging() { return 0; };
    virtual HttpAnswer httpSend(HttpMethod method, const NString& url, NString& payload,
                                const std::vector<HttpHeader>& headers, uint16_t timeout) {
        (void)method;
        (void)url;
        (void)payload, (void)headers;
        (void)timeout;
        return {0, nullptr};
    };
    virtual RenderTarget* GetScreen() { return nullptr; };
};
#ifdef PC
#include <chrono>
#include <sys/time.h>
#include <time.h>

inline void analogWrite(int pin, int value) {
    // No-op on PC
    (void)pin;
    (void)value;
}

inline int constrain(int value, int min, int max) {
    if (value < min) { return min; }
    if (value > max) { return max; }
    return value;
}

inline int settimeofday(const struct timeval* tv, const void* tz) {
    (void)tv;
    (void)tz;
    return 0; // Stub for PC
}

#define TFT_BL 4 // Dummy value for PC

#else
#include <Arduino.h>
#endif