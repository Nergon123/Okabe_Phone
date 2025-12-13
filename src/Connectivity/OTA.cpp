#include "OTA.h"
#ifdef ARDUINO
WebServer server(80);
WebOTA    ota(server);
#endif
void OTAactivity() {
#ifdef ARDUINO
    if (WiFi.status() != WL_CONNECTED) {
        ErrorWindow("WiFi is Not Connected");
        return;
    };
    // Set device info for OTA update
    tft.fillScreen(0);
    ota.setDeviceInfo(String(FIRMVER), "OkabePhone");
    tft.setCursor(0, 50);
    tft.setTextColor(TFT_WHITE);
    tft.setTextFont(0);
    tft.printf("WAITING FOR OTA\nhttp://%s/update", WiFi.localIP().toString().c_str());
    // Callbacks
    ota.onStart([](const String &filename) {
        tft.fillScreen(0);
        bootText(NString("Start updating ") + NString(filename.c_str()));
    });
    bool once = false ;ota.onProgress([&](size_t current, size_t total) {
        if (!once) {
            bootText("Updating...",-1,180);
            once = true;
        }
        progressBar((int)current, (int)total, 230, 8, TFT_WHITE, false, true);
    });
    ota.onEnd([]() { bootText("Update finished"); });
    ota.onError([](int err) { bootText("[OTA] Error: %d\n", err); });

    ota.begin();
    server.begin();
    while (true) {
        server.handleClient();
        if (buttonsHelding(false) == BACK) { return; }
    }
#else
    ErrorWindow("Not supported on current platform...");
#endif // ARDUINO
}