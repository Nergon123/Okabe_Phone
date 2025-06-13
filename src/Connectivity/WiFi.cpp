#include "_WiFi.h"
#include "../System/Generic.h"

// TODO: Implement WiFi functionality
// Why do I even need wifi here?
void WifiConnect(String ssid, String password) {
}

// List available WiFi networks
void WiFiList() {
    
    WiFi.begin();
    WiFi.mode(WIFI_STA);
    while (true) {
        int      count = WiFi.scanNetworks();
        String   names[50];
        uint8_t  a;
        uint8_t *l;
        int32_t  c, d;
        for (int i = 0; i < count; i++) {
            WiFi.getNetworkInfo(i, names[i], a, c, l, d);
        }
        int ch = listMenu(names, count, false, 0, "WI-FI");
        if (ch == -1)
            return;
        else {
            // TODO: Actually Connect to WiFi

            return;
        }
    }
}



