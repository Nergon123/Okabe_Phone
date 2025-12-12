#include "Generic.h"
#include "../Platform/ESP32Memory.h"
/*
 * Get the current charge level of the battery
 * @return `int`: 0-3 of 3
 */
int getChargeLevel() {
    if (!ip5306exists) { return 3; }
    int toIcon = (hw->getBatteryCharge() / 25) - 1;

    toIcon = (toIcon < 0) ? 0 : (toIcon > 3 ? 3 : toIcon);

    return toIcon;
}

ulong funcTime = 0;
void  GetFuncTime(bool start, const char *who = "UNKNOWN") {
#ifdef DEVMODE
    if (start) { funcTime = hw->micros(); }
    else {
        ESP_LOGD("PERFOMANCE", "\n%s run for %.3f ms\n", who,
                 (float)(hw->micros() - funcTime) / 1000);
    }
#endif
}

// Set frequencies to fast or slow mode
// @param status: true for fast mode, false for slow mode
void fastMode(bool status) { hw->setCPUSpeed(status ? CPU_SPEED::CPU_FAST : CPU_IDLE); }

uint8_t currentBrightness = 70; // initial
uint8_t targetBrightness  = 70;

void setBrightnessTask(void *) {
    for (;;) {
        if (currentBrightness != targetBrightness) {
            if (targetBrightness > currentBrightness) { currentBrightness++; }
            else { currentBrightness--; }

            hw->setScreenBrightness(currentBrightness);
        }
        hw->delay(5);
    }
}

void setBrightness(uint8_t percentage) {
    targetBrightness = constrain(percentage, 0, 100);
}
