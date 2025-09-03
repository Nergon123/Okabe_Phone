#include "GlobalVariables.h"
#include "Screens/Main.h"
#include "System/ResourceSystem.h"
#include "System/Time.h"

#include "System/Tasks.h"
#include "esp_task_wdt.h"
#include "init.h"
void restart_handler() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_RED);
    tft.setCursor(0, 0);
    tft.println("RESTARTING...");
    delay(100);
}

// initialize the system
void setup() {
    esp_task_wdt_deinit();
    esp_task_wdt_init(10000, false);
    esp_register_shutdown_handler(restart_handler);

    SetUpTime();
    hardwareInit();
    if (chrg.isChargerConnected() == 1) {
        offlineCharging();
    }
    tft.fillScreen(0x0000);
    tft.setTextFont(1);
    tft.setCursor(0, 0);
    progressBar(0, 100, 250);
    if(!psramFound()){
        changeFont(2);
        tft.setTextColor(TFT_RED);
        tft.setCursor(0,50);
        tft.print("PSRAM NOT FOUND!!!\nEXPECT PROBLEMS!!!");
    }
    storageInit();

    if (buttonsHelding(false) == '*') {
        recovery("Manually triggered recovery."); // Chance to change resource file to custom one
    }

    res.CopyToRam();
    res.DrawImage(R_BOOT_LOGO);
    bootText("Initializing RTOS tasks...");
    initTasks();
    ESP_LOGI("DEVICE",
             "\nOkabePhone " FIRMVER "\n\n Phone firmware written by Nergon\n\n "
             "Resources located in sdcard (%s)\n",
             resPath.c_str());
    ESP_LOGI("DEVICE", "%s REV.%u %u MHz %d cores", ESP.getChipModel(), ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getChipCores());
    
    progressBar(100, 100, 250);
    millSleep = millis();
    
    Serial.updateBaudRate(SERIAL_BAUD_RATE);
    fastMode(false);
    if (buttonsHelding(false) == '#')
        AT_test();


    // changeFont(3);
    // tft.fillScreen(tft.color24to16(0x00008b));
    // tft.setTextColor(0xF800);
    // tft.drawNumber(9292929292929, 150, 1);
    // for (;;) ;

}

// Function to handle the main loop
void loop() {
    screens();
}