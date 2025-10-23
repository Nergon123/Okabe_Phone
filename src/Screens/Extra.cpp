#include "Extra.h"

/*
 * ## Executes application from sdcard
 *
 *  Not really an "application", but more like second bootable image.
 *  This thing just flashes image into second partition and boots it from it.
 *
 */
void execute_application() {
#ifndef PC
    suspendCore(true);
    NString file_path = fileBrowser("/", "bin");
    if (strcmp(file_path.c_str(), "")) { return; }
    tft.fillScreen(0);
    mOption mOp[2] = {{"Yes"}, {"No"}};
    int     choice = listMenuNonGraphical(
        mOp, 2, "You are going to launch \"" + file_path + "\"! Are you sure about that?");
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
    NFile* file = VFS.open(file_path.c_str(), FILE_READ);
    if (!file) {
        ESP_LOGE("ERROR","Failed to open file!");
        return;
    }

    const esp_partition_t* partition =
        esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, "app1");
    if (!partition) {
        ESP_LOGE("ERROR","Partition not found!");
        return;
    }

    if (file->size() > partition->size) {
        tft.setCursor(0, 210);
        tft.printf("File size bigger than partition size (%d bytes/%d bytes)\n\n", file->size(),
                   partition->size);

        return;
    }
    esp_partition_erase_range(partition, 0, partition->size);

    uint8_t buffer[4096];
    size_t  offset = 0;
    while (file->available()) {
        size_t bytesRead = sizeof(buffer);
        file->read(buffer, sizeof(buffer));
        esp_partition_write(partition, offset, buffer, bytesRead);
        progressBar(offset, file->size(), 200);
        offset += bytesRead;
    }

    file->close();
    esp_ota_set_boot_partition(partition);
    ESP.restart();
#endif
}

// Additional features screen
void e() {

    int choice = -2;
    while (choice != -1) {
        NString debug[] = {"Delete File", "View Text File content", "Execute Application",
                          "Set Date/Time", "Connect To Wi-Fi"};
        choice         = listMenu(debug, ArraySize(debug), false, 2, "Additional Features");
        NString path;
        NFile* file;
        switch (choice) {
        case 0:
            path = fileBrowser();
            ESP_LOGI("FILE", "DELETE %s", path.c_str());
            VFS.remove(path);
            break;
        case 1:
            // NOT IMPLEMENTED
            break;
        case 2: execute_application(); break;
        case 3: setTime(&systemTime); break;
        case 4: WiFiList(); break;
        }

        break;
    }
    currentScreen = SCREENS::MAINMENU;
}