#include "../System/Generic.h"
#include "../UI/UIElements.h"
#include "_WiFi.h"
// TODO: Implement WiFi functionality
// Why do I even need wifi here?

void WifiPrompt(String ssid, uint8_t encryptionType) {
    return;
    // TODO
    String password;
    res.DrawImage(R_LIST_MENU_BACKGROUND);
    res.DrawImage(R_LIST_HEADER_BACKGROUND);
    res.DrawImage(R_LIST_HEADER_ICONS, 2);
    int  choice    = 0;
    int  direction = -1;
    bool exit      = false;
    bool connect   = false;
    bool open      = encryptionType == WIFI_AUTH_OPEN;
    while (choice != -1) {
        ssid = InputField("SSID", ssid, 40, 0, 0, choice == 0, &direction);
        if (!open) {
            password = InputField("Password", password, 40, 0, 0, choice == 1, &direction);
        }
        else if (choice == 1) { choice = 2; }

        connect = button("Connect", 10, 280, 100, 30, choice == 2, &direction);
        exit    = button("Cancel", 120, 280, 100, 30, choice == 3, &direction);
    }
}

void WifiConnect(String ssid, String password) { WiFi.begin(ssid, password); }

// List available WiFi networks
void WiFiList() {
    WiFi.begin();
    WiFi.mode(WIFI_STA);
    while (true) {
        int      count = WiFi.scanNetworks();
        String   names[count];
        uint8_t  enc[count];
        uint8_t *l;
        int32_t  c, d;
        for (int i = 0; i < count; i++) { WiFi.getNetworkInfo(i, names[i], enc[i], c, l, d); }

        int ch = listMenu(names, count, false, 0, "WI-FI");
        if (ch == -1) { return; }
        else {
            // TODO: Actually Connect to WiFi
            WifiPrompt(names[ch], enc[ch]);
            return;
        }
    }
}
