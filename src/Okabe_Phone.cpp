#include "GlobalVariables.h"
#include "Screens/Main.h"
#include "System/Time.h"
#include "images.h"
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

    if (buttonsHelding(false) == '*') {
        recovery("Manually triggered recovery."); // Chance to change resource file to custom one
    }
    storageInit();
    drawImage(50, 85, LOGO_IMAGE);
    initTasks();
    ESP_LOGI("DEVICE", "\nOkabePhone " FIRMVER "\n\nPhone firmware written by Nergon\n\nResources located in sdcard (%s)\n", resPath.c_str());
    ESP_LOGI("DEVICE", "%s REV.%u %u MHz %d cores", ESP.getChipModel(), ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getChipCores());
    progressBar(100, 100, 250);
    millSleep = millis();
    Serial.updateBaudRate(SERIAL_BAUD_RATE);
    
    if (buttonsHelding(false) == '#')
        AT_test();
    messageActivityOut(Contact("Assistant","+0000000000"));
}

// Function to handle the main loop
void loop() {
    screens();
}