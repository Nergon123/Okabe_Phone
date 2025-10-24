#pragma once

#ifdef PC
#include <chrono>
#include <sys/time.h>
#include <time.h>

inline unsigned long micros() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto duration = duration_cast<microseconds>(now.time_since_epoch());
    return duration.count();
}

inline void analogWrite(int pin, int value) {
    // No-op on PC
    (void)pin; (void)value;
}

inline int constrain(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}


inline int settimeofday(const struct timeval* tv, const void* tz) {
    (void)tv; (void)tz;
    return 0; // Stub for PC
}

inline unsigned int getCpuFrequencyMhz() {
    return 240; // Default value for PC
}

inline void setCpuFrequencyMhz(unsigned int freq) {
    (void)freq;
    // No-op on PC
}

#define TFT_BL 4 // Dummy value for PC

#else
#include <Arduino.h>
#endif