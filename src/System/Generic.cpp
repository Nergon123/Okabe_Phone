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
        ESP_LOGD("PERFOMANCE", "\n%s run for %.3f ms\n", who, (float)(hw->micros() - funcTime) / 1000);
    }
#endif
}

// Set frequencies to fast or slow mode
// @param status: true for fast mode, false for slow mode
void fastMode(bool status) { hw->setCPUSpeed(status ? CPU_SPEED::CPU_FAST : CPU_IDLE); }

uint8_t currentBrightness = brightness;
// Set screen brightness
// @param percentage: brightness percentage (0-100)
void setBrightness(uint8_t percentage) {
    constrain(percentage, 0, 100);
    if (currentBrightness != percentage) {
        if (percentage > currentBrightness) {
            // smooth brightness change
            for (int i = currentBrightness; i <= percentage; i++) {
                hw->setScreenBrightness(i);
                hw->delay(5);
            }
        }
        else {
            for (int i = currentBrightness; i >= percentage; i--) {
                hw->setScreenBrightness(i);
                hw->delay(5);
            }
        }
    }
    currentBrightness = percentage;
}
