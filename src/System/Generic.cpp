#include "Generic.h"
/*
 * Get the current charge level of the battery
 * @return `int`: 0-3 of 3
 */
int getChargeLevel() {

    int toIcon = (chrg.getBatteryLevel() / 25) - 1;

    toIcon = (toIcon < 0) ? 0 : (toIcon > 3 ? 3 : toIcon);

    return toIcon;
}

ulong funcTime = 0;
#ifdef DEVMODE
void GetFuncTime(bool start, const char *who = "UNKNOWN") {
    if (start) {
        funcTime = micros();
    } else {
        Serial.printf("\n%s run for %.3f ms\n", who, (float)(micros() - funcTime) / 1000);
    }
}
#endif

// Set frequencies to fast or slow mode
// @param status: true for fast mode, false for slow mode
void fastMode(bool status) {
    if ((status && getCpuFrequencyMhz() == FAST_CPU_FREQ_MHZ) || (!status && getCpuFrequencyMhz() == SLOW_CPU_FREQ_MHZ)) {
        // No need to change frequency
        return;
    }
    setCpuFrequencyMhz(status ? FAST_CPU_FREQ_MHZ : SLOW_CPU_FREQ_MHZ);
    Serial.updateBaudRate(SERIAL_BAUD_RATE);
    SimSerial.updateBaudRate(SIM_BAUD_RATE);
    if (!isSPIFFS)
        initSDCard(status);
}

int currentBrightness = brightness;
// Set screen brightness
// @param percentage: brightness percentage (0-100)
void setBrightness(int percentage) {
    percentage = constrain(percentage, 0, 100);
    if (currentBrightness != percentage) {
        if (percentage > currentBrightness) {
            // smooth brightness change
            for (int i = currentBrightness; i <= percentage; i++) {
                analogWrite(TFT_BL, (256 * i) / 100);
                delay(5);
            }
        } else {
            for (int i = currentBrightness; i >= percentage; i--) {
                analogWrite(TFT_BL, (256 * i) / 100);
                delay(5);
            }
        }
    }
    currentBrightness = percentage;
}
