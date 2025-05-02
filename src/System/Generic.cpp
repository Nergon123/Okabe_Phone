#include "Generic.h"
// Get battery charge (from 0 to 3)
int getChargeLevel() {

    int toIcon = (chrg.getBatteryLevel() / 25) - 1;

    toIcon = (toIcon < 0) ? 0 : (toIcon > 3 ? 3 : toIcon);

    return toIcon;
}

void fastMode(bool status) {
    // TODO: REINIT SCREEN and SDCARD
    setCpuFrequencyMhz(status ? FAST_CPU_FREQ_MHZ : SLOW_CPU_FREQ_MHZ);
    Serial.updateBaudRate(115200);
    if (!isSPIFFS)
        initSDCard(status);
}

int currentBrightness = brightness;
// Set screen brightness
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
