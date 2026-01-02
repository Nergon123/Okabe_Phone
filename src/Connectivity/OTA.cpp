#include "OTA.h"
#ifdef ARDUINO
WebServer server(80);
WebOTA    ota(server);

void WebOTATask(void* param) {
    WebServer* server = (WebServer*)param;
    while (true) {
        server->handleClient();
        vTaskDelay(1); // yield to watchdog
    }
    vTaskDelete(NULL);
}
#endif

void OTAactivity() {
#ifdef ARDUINO
    if (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_MODE_AP &&
        WiFi.getMode() != WIFI_MODE_APSTA) {
        InfoWindow("WiFi is Not Connected");
        return;
    }

    tft.fillScreen(0);
    ota.setDeviceInfo(String(FIRMVER), PRODUCTNAME);
    tft.setCursor(0, 50);
    tft.setTextColor(TFT_WHITE);
    changeFont(0);
    NString WiFIIP = "";
    if (WiFi.localIP()[0] != 0) { WiFIIP = WiFi.localIP().toString().c_str(); }
    else if (WiFi.softAPIP()[0] != 0) { WiFIIP = WiFi.softAPIP().toString().c_str(); }

    tft.printf("WAITING FOR OTA\nhttp://%s/update\nor\nhttp://%s/update", WiFIIP.c_str(),
               WiFi.getHostname());
    currentRenderTarget->present();
    // Callbacks
    ota.onStart([](const String& filename) {
        tft.fillScreen(0);
        tft.setCursor(0, 50);
        tft.printf("Update Started %s\n", filename.c_str());
        currentRenderTarget->present();
    });
    ota.onProgress([&](size_t current, size_t total) {
        progressBar((int)current, (int)total, 230, 8, TFT_WHITE, false, true);
    });
    ota.onEnd([]() {
        tft.setTextColor(TFT_GREEN);
        tft.printf("Update Finished\n");
        currentRenderTarget->present();
    });
    ota.onError([](int err) {
        tft.setTextColor(TFT_RED);
        tft.printf("OTA ERROR %d\n", err);
        tft.setTextColor(TFT_WHITE);
        currentRenderTarget->present();
    });

    ota.begin();
    server.begin();

    // This thing needs more stack
    TaskHandle_t* update = NULL;
    xTaskCreate(WebOTATask, "WebOTATask", 16384, &server, 1, update);

    while (true) {
        if (buttonsHelding(false) == BACK) {
            if (update /*what if it will be null and the whole application will be killed T_T*/) {
                vTaskDelete(update);
            }
            break;
        }
        delay(1); // yield
    }
#else
    InfoWindow("Not supported on current platform...");
#endif // ARDUINO
}
