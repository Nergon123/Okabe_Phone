#include "GlobalVariables.h"
#include "Screens/Main.h"
#include "System/ResourceSystem.h"
#include "System/Time.h"

#include "System/Tasks.h"
#include "esp_task_wdt.h"
#include "init.h"
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
    esp_task_wdt_deinit();
    esp_task_wdt_init(10000, false);
    esp_register_shutdown_handler(shutdown_handler);

    SetUpTime();
    hardwareInit();

    if (ip5306exists && chrg.isChargerConnected()) { offlineCharging(); }

    tft.fillScreen(0x0000);
    tft.setTextFont(1);
    tft.setCursor(0, 0);
    progressBar(0, 100, 250);

    if (!psramFound()) {
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(0, 50);
        tft.print("psram not found...");
    }
    storageInit();

    // Chance to change resource file to custom one
    if (buttonsHelding(false) == '*') { recovery("Manually triggered recovery."); }

    res.CopyToRam();
    res.DrawImage(R_BOOT_LOGO);
    bootText("Initializing RTOS tasks...");
    initTasks();

    ESP_LOGI("DEVICE",
             "\nOkabePhone " FIRMVER
             "\n\n Phone firmware written by Nergon123 and contributors\n\n "
             "Resources located in sdcard (%s)\n",
             resPath.c_str());

    ESP_LOGI("DEVICE", "%s REV.%u %u MHz %d cores", ESP.getChipModel(), ESP.getChipRevision(),
             ESP.getCpuFreqMHz(), ESP.getChipCores());

    progressBar(100, 100, 250);

    Serial.updateBaudRate(SERIAL_BAUD_RATE);

    if (buttonsHelding(false) == '#') { AT_test(); }

    mOption mOptions[] = {
        mOption{.label = "0", .image = Image(R_FILE_MANAGER_ICONS), .icon_index = 0},
        mOption{.label = "1", .image = Image(R_FILE_MANAGER_ICONS), .icon_index = 1},
        mOption{.label = "2", .image = Image(R_FILE_MANAGER_ICONS), .icon_index = 2},
        mOption{.label = "3", .image = Image(R_FILE_MANAGER_ICONS), .icon_index = 3},
        mOption{.label = "4", .image = Image(R_FILE_MANAGER_ICONS), .icon_index = 4},
        mOption{.label = "5", .image = Image(R_FILE_MANAGER_ICONS), .icon_index = 5},
        mOption{.label = "6", .image = Image(R_FILE_MANAGER_ICONS), .icon_index = 6},
        mOption{.label = "7", .image = Image(R_FILE_MANAGER_ICONS), .icon_index = 7},

    };
    millSleep = millis();
    listMenu(mOptions, 8, false, 0,"SAMPLE");
}

// Function to handle the main loop
void loop() { screens(); }