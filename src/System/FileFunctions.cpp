#include "FileFunctions.h"
const char* FTAG = "FileFunc";
bool        FileOpen(NFile file) {
    String fileFormat =
        String(file.file.name()).substring(String(file.file.name()).lastIndexOf('.'));
    for (int i = 0; i < sizeof(funcFormats) / sizeof(funcFormats[0]); i++) {
        if (String(funcFormats[i].format).indexOf(fileFormat) >= 0) {
            return funcFormats[i].callback(file);
        }
    }
    return NotSupported(file);
}
bool NotSupported(NFile file) {
    ESP_LOGI(FTAG, "File not supported");

    // maybe some context window
    return false;
}
bool ViewText(NFile file) {
    ESP_LOGI(FTAG, "ViewText Not Implemented");
    return false;
}
bool ViewPNG(NFile file) {
    ESP_LOGI(FTAG, "ViewPNG Not Implemented");
    return false;
}
bool ApplyTheme(NFile file) {
    if (!confirmation("Do you want to apply theme \n\"" + String(file.file.name()) + "\"?")) {
        return false;
    };
    tft.fillScreen(0);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(0, 30);
    tft.print("wait...");
    res.Init(file.file);
    res.CopyToRam(RES_MAIN);
    tft.fillScreen(0);
    drawStatusBar(true);
    return false;
}