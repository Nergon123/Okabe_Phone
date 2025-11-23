#include "GlobalVariables.h"
#include "Platform/Hardware/Hardware.h"
#include "Platform/Hardware/Profiles/ESP32.h"
#include "Platform/Hardware/Profiles/Linux.h"
#include "Screens/Main.h"
#include "System/ResourceSystem.h"
#include "System/Tasks.h"
#include "System/Time.h"
#include "init.h"
bool err = false;

int start() {
#ifdef PC
    hw = new DEV_LINUX();
#else
    hw = new DEV_ESP32();
#endif
    SetUpTime();
    // hardwareInit();
    hw->init();
    currentRenderTarget = hw->GetScreen();
    ESP_LOGI("INIT", "Main render target %p", currentRenderTarget);
    if (!currentRenderTarget) {

        ESP_LOGE("INIT", "Error occurred when initializing screen (GetScreen returned nullptr)");
        return 2;
    }
    tft.setRenderTarget(currentRenderTarget);
    if (hw->isCharging()) { offlineCharging(); }

    tft.fillScreen(0x0000);
    tft.setTextFont(1);
    tft.setCursor(0, 0);
    progressBar(0, 100, 250);

    // Chance to change resource file to custom one
    storageInit();
    if (buttonsHelding(false) == '*') { recovery("Manually triggered recovery."); }

    res.CopyToRam();
    res.DrawImage(R_BOOT_LOGO);
    bootText("Initializing RTOS tasks...");
    initTasks();

    ESP_LOGI("DEVICE",
             "\nOkabePhone " FIRMVER
             "\n\n Phone firmware written by Nergon123 and contributors\n\n "
             "Resources located in %s\n",
             resPath.c_str());

    ESP_LOGI("DEVICE", "%s", hw->getDeviceName());

    progressBar(100, 100, 250);

    if (buttonsHelding(false) == '#') { AT_test(); }

    millSleep = hw->millis();
    return 0;
}

void setup() { start(); }
// Function to handle the main loop
void loop() { screens(); }

#ifdef PC
int main(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--scale") == 0) {
            if (argv[i + 1]) {
                SDLScale = atoi(argv[i + 1]);
                if (SDLScale < 1 || SDLScale > 9) {
                    printf("\n--scale value should be in range from 1 to 9\n");
                    return -1;
                }

                SDLScale = SDLScale > 0 ? SDLScale : 1;
                i++;
            }
            else {
                printf("--scale needs value from 1 to 9");
                return -1;
            }
        }
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Okabe Phone " FIRMVER "\n" REPOSITORY_LINK "\n"
                   "\n\t--help, -h         Show this screen"
                   "\n\t--scale n, -s n    Multiply window size by n times"
                   "\n\n");
            return 0;
        }
    }
    int err = start();
    if (err != 0) { return err; }
    while (true) { loop(); }
    return 0;
}
#endif