#include <chrono>
#include <thread>
#include "Defines.h"
#include "ard_esp.h"

#ifdef PC
unsigned long hw->millis() {
    using namespace std::chrono;
    // Get current time point
    auto now = system_clock::now();
    // Convert to milliseconds since epoch
    auto duration = duration_cast<milliseconds>(now.time_since_epoch());
    return duration.count();
}

void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Serial instances for PC
SerialClass Serial;
SerialClass Serial1;

#else

extern void delay(unsigned long ms);
#endif