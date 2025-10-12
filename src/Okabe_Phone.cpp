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

    ESP_LOGI("DEVICE", "%s REV.%u %u MHz %d cores", ESP.getChipModel(), ESP.getChipRevision(),
             ESP.getCpuFreqMHz(), ESP.getChipCores());

    progressBar(100, 100, 250);

    Serial.updateBaudRate(SERIAL_BAUD_RATE);

    if (buttonsHelding(false) == '#') { AT_test(); }

    millSleep = millis();

}

// Function to handle the main loop
void loop() { screens(); }