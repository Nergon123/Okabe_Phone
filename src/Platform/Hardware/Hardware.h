#pragma once
#include "stdlib.h"
#include <Platform/Graphics/RenderTargets.h>
enum CPU_SPEED { CPU_IDLE, CPU_SLOW, CPU_DEFAULT, CPU_FAST, CPU_FULL, CPU_TURBO };

class iHW {
  public:
    virtual void  init() {};
    virtual void  initStorage() {};
    virtual ulong micros() { return 0; };
    virtual ulong millis() { return micros() / 1000; };
    virtual void  delay(ulong ms) {
        ulong mil = millis();
        while (millis() < mil + ms);
    }
    virtual void        setCPUSpeed(CPU_SPEED speed) { (void)speed; };
    virtual CPU_SPEED   getCPUSpeed() { return CPU_DEFAULT; };
    virtual void        updateFrequencies() {};
    virtual const char* getDeviceName() { return "Unknown"; };
    virtual void        shutdown() {};
    virtual void        reboot() {};

    virtual void setScreenBrightness(int8_t value) { (void)value; };
    virtual char getCharInput() { return 0; };
    virtual int  getKeyInput() { return 0; };
    virtual int  getBatteryCharge() { return 3; };
    virtual bool isCharging() { return 0; };

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