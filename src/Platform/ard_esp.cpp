#include <chrono>
#include <thread>
#include "Defines.h"
#include "ard_esp.h"

#ifdef PC
void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
#else

extern void delay(unsigned long ms);
#endif