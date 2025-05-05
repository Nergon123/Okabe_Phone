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

// Set frequencies to fast or slow mode
// @param status: true for fast mode, false for slow mode
void fastMode(bool status) {
    if ((status && getCpuFrequencyMhz() == FAST_CPU_FREQ_MHZ) || (!status && getCpuFrequencyMhz() == SLOW_CPU_FREQ_MHZ)) {
        // No need to change frequency
        return;
    }
    setCpuFrequencyMhz(status ? FAST_CPU_FREQ_MHZ : SLOW_CPU_FREQ_MHZ);
    Serial.updateBaudRate(115200);
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

void printT_S(String str) {

    ESP_LOGI("BOOT", "%s", str.c_str());
#ifdef LOG
    tft.println(str);
#endif
}
