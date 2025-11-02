#include "GlobalVariables.h"
#include "Platform/ESPPlatform.h"
#include "Platform/ard_esp.h"
#include "Screens/Main.h"
#include "System/ResourceSystem.h"
#include "System/Time.h"

#include "System/Tasks.h"
#include "init.h"

#ifndef PC
#include "esp_task_wdt.h"
#else

void setup();
void loop();

int main(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--scale") == 0) {
            if (argv[i + 1]) {
                if (strlen(argv[i + 1]) > 1 || argv[i + 1][0] <= '0' || argv[i + 1][0] > '9') {
                    printf("\n--scale value should be in range from 1 to 9\n");
                    return -1;
                }

                SDLScale = atoi(argv[i + 1]);
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
    setup();
    while (true) { loop(); }
    return 0;
}
#endif
void shutdown_handler() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_RED);
    tft.setCursor(0, 0);
    tft.println("Shutting down...");
    delay(100);
}

// initialize the system
void setup() {
#ifndef PC
    esp_task_wdt_deinit();
    esp_task_wdt_init(10000, false);
    esp_register_shutdown_handler(shutdown_handler);
#endif

    SetUpTime();
    hardwareInit();

    
    if (ip5306exists && chrg.isChargerConnected()) { offlineCharging(); }

    tft.fillScreen(0x0000);
    tft.setTextFont(1);
    tft.setCursor(0, 0);
    progressBar(0, 100, 250);

    // Chance to change resource file to custom one
    if (buttonsHelding(false) == '*') { recovery("Manually triggered recovery."); }
    storageInit();

    res.CopyToRam();
    res.DrawImage(R_BOOT_LOGO);
    bootText("Initializing RTOS tasks...");
    initTasks();

    ESP_LOGI("DEVICE",
             "\nOkabePhone " FIRMVER
             "\n\n Phone firmware written by Nergon123 and contributors\n\n "
             "Resources located in sdcard (%s)\n",
             resPath.c_str());

    ESP_LOGI("DEVICE", "PC Build");

    progressBar(100, 100, 250);

    Serial.updateBaudRate(SERIAL_BAUD_RATE);

    if (buttonsHelding(false) == '#') { AT_test(); }

    millSleep = millis();
}

// Function to handle the main loop
void loop() { screens(); }