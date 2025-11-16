#pragma once
#include "stdlib.h"
enum CPU_SPEED { CPU_IDLE, CPU_SLOW, CPU_DEFAULT, CPU_FAST, CPU_FULL, CPU_TURBO };

class iHW {
  public:
    virtual void        init();
    virtual ulong       micros();
    virtual ulong       millis(){ return micros() / 1000;};
    virtual void        setCPUSpeed(CPU_SPEED speed);
    virtual CPU_SPEED   getCPUSpeed();
    virtual void        updateFrequencies();
    virtual const char* getDeviceName();
    virtual void        shutdown();
    virtual void        reboot();

    virtual void setScreenBrightness(int8_t value);
    virtual char getCharInput();
    virtual int getKeyInput();
    virtual int  getBatteryCharge();
    virtual bool isCharging();

    virtual RenderTarget* GetScreen();
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