#include "Extra.h"

/*
 * ## Executes application from sdcard
 *
 *  Not really an "application", but more like second bootable image.
 *  This thing just flashes image into second partition and boots it from it.
 *
 */
void execute_application() {
    
    suspendCore(true);
    String file_path = fileBrowser(SD.open("/"), "bin");
    tft.fillScreen(0);
    mOption mOp[2] = {{"Yes"}, {"No"}};
    int     choice = listMenuNonGraphical(mOp, 2, "DO YOU REALLY TRUST THIS APPLICATION?\n\nPATH:sdcard:" + file_path + "\n\nNOTICE: WITH APPLICATION ACTIVE\n\n YOU WOULDN'T RECIEVE ANY\n\n CALLS MESSAGES ETC.\n\n(Unless application designed to)\n\n BY LAUNCHING APPLICATION YOU\n\nGIVING ACCESS TO WHOLE DEVICE\n\n INCLUDING:\n\n SD CARD, SIM CARD,WIFI,\n\n BLUETOOTH,FIRMWARE etc.\n\n\n Continue?");
    if (choice) {
        sBarChanged = true;
        drawStatusBar();
        suspendCore(false);
        return;
    }
    tft.fillScreen(0);
    changeFont(0);
    tft.setTextColor(0xFFFF);
    tft.setCursor(30, 190);
    tft.println("BOOTING INTO APPLICATION...");
    if (file_path == "/null")
        return;
    
    File file = SD.open(file_path);
    if (!file) {
        Serial.println("Failed to open file!");
        return;
    }

    const esp_partition_t *partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, "app1");
    if (!partition) {
        Serial.println("Partition not found!");
        return;
    }

    if (file.size() > partition->size) {
        tft.setCursor(0, 210);
        tft.printf("File size bigger than partition size (%d bytes/%d bytes)\n\n", file.size(), partition->size);
        
        return;
    }
    esp_partition_erase_range(partition, 0, partition->size);

    uint8_t buffer[4096];
    size_t  offset = 0;
    while (file.available()) {
        size_t bytesRead = sizeof(buffer);
        file.read(buffer, sizeof(buffer));
        esp_partition_write(partition, offset, buffer, bytesRead);
        progressBar(offset, file.size(), 200);
        offset += bytesRead;
    }

    file.close();
    esp_ota_set_boot_partition(partition);
    ESP.restart();
}

// Additional features screen
void e() {

    int choice = -2;
    while (choice != -1) {
        String debug[] = {
            "Delete File",
            "Cat",
            "PC Connection",
            "Execute Application",
            "Set Date/Time",
            "Connect To Wi-Fi"};
        choice = listMenu(debug, ArraySize(debug), false, 2, "Additional Features");
        String path;
        File   file;
        switch (choice) {
        case 0:
            path = fileBrowser();
            SD.remove(path);
            break;
        case 1:
            path = fileBrowser();
            tft.fillScreen(0);
            tft.setCursor(0, 0);
            changeFont(0);
            tft.setTextColor(TFT_WHITE);
            file = SD.open(path);
            file.seek(0);
            if (file)
                while (file.available()) {
                    tft.print((char)file.read());
                }
            file.close();
            delay(1000);
            while (buttonsHelding() == -1)
                ;
            sBarChanged = true;
            drawStatusBar();
            break;
        case 2:
            SerialConnection();
            break;
        case 3:
            execute_application();
            break;
        case 4:
            setTime(&systemTime);
            break;
        case 5:
            WiFiList();
            break;
        }

        break;
    }
    currentScreen = SCREENS::MAINMENU;
}