#include "Generic.h"
#include "../Platform/Hardware.h"
#include "../Platform/ESP32Memory.h"
/*
 * Get the current charge level of the battery
 * @return `int`: 0-3 of 3
 */
int getChargeLevel() {
    if (!ip5306exists) { return 3; }
    int toIcon = (chrg.getBatteryLevel() / 25) - 1;

    toIcon = (toIcon < 0) ? 0 : (toIcon > 3 ? 3 : toIcon);

    return toIcon;
}

ulong funcTime = 0;
void  GetFuncTime(bool start, const char *who = "UNKNOWN") {
#ifdef DEVMODE
    if (start) { funcTime = micros(); }
    else { ESP_LOGD("PERFOMANCE","\n%s run for %.3f ms\n", who, (float)(micros() - funcTime) / 1000); }
#endif
}

// Set frequencies to fast or slow mode
// @param status: true for fast mode, false for slow mode
void fastMode(bool status) {
    if ((status && getCpuFrequencyMhz() == FAST_CPU_FREQ_MHZ) ||
        (!status && getCpuFrequencyMhz() == SLOW_CPU_FREQ_MHZ)) {
        return;
    }
    setCpuFrequencyMhz(status ? FAST_CPU_FREQ_MHZ : SLOW_CPU_FREQ_MHZ);
    Serial.updateBaudRate(SERIAL_BAUD_RATE);
    SimSerial.updateBaudRate(SIM_BAUD_RATE);
    initSDCard(status);
}

uint8_t currentBrightness = brightness;
// Set screen brightness
// @param percentage: brightness percentage (0-100)
void setBrightness(uint8_t percentage) {
    percentage = percentage > 100 ? 100 : percentage; //;constrain(percentage, 0, 100);
    if (currentBrightness != percentage) {
        if (percentage > currentBrightness) {
            // smooth brightness change
            for (int i = currentBrightness; i <= percentage; i++) {
                analogWrite(TFT_BL, (256 * i) / 100);
                delay(5);
            }
        }
        else {
            for (int i = currentBrightness; i >= percentage; i--) {
                analogWrite(TFT_BL, (256 * i) / 100);
                delay(5);
            }
        }
    }
    currentBrightness = percentage;
}
