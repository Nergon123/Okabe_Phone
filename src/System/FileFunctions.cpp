#include "FileFunctions.h"
const char* FTAG = "FileFunc";

#warning FileFunctions not implemented
bool        FileOpen(NFile* file) {
    (void)file;
    return false;
}
bool NotSupported(NFile* file) {
    ESP_LOGI(FTAG, "File not supported");
    (void)file;
    // maybe some context window
    return false;
}
bool ViewText(NFile* file) {
    ESP_LOGI(FTAG, "ViewText Not Implemented");
    (void)file;
    return false;
}
bool ViewPNG(NFile* file) {
    ESP_LOGI(FTAG, "ViewPNG Not Implemented");
    (void)file;
    return false;
}
bool ApplyTheme(NFile* file) {
    // if (!confirmation("Do you want to apply theme \n\"" + file->name() + "\"?")) {
    //     return false;
    // };
    tft.fillScreen(0);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(0, 30);
    tft.print("wait...");
    res.Init(file);
    res.CopyToRam(RES_MAIN);
    tft.fillScreen(0);
    drawStatusBar(true);
    return false;
}