#include "System/Generic.h"
#include "UI/UIElements.h"
#include "_WiFi.h"

#ifdef WiFi_h
void WifiPrompt(NString ssid, uint8_t encryptionType, NString password = NString()) {
    std::vector<FIELD> fields = {
        FIELD("Name", ssid, false),
        FIELD("Password", password, false),
    };
    if (InputFieldS("Connect To Wi-Fi", fields, LM_SETTINGS, 0, "Connect", "Cancel")) {
        WiFi.begin(ssid.c_str(), password.c_str());
    }
}
#endif
// List available WiFi networks
void WiFiList() {
#ifdef WiFi_h
    if (WiFi.getMode() == WIFI_MODE_STA || WiFi.getMode() == WIFI_MODE_APSTA) {
        while (true) {
            int count = WiFi.scanNetworks();
            if (count == 0) { return; }
            uint8_t              enc[count];
            uint8_t             *l;
            int32_t              RSSI, d;
            std::vector<mOption> list;
            for (int i = 0; i < count; i++) {
                String name;
                WiFi.getNetworkInfo(i, name, enc[i], RSSI, l, d);
                ESP_LOGI("WI-FI", "RSSI %d", RSSI);
                uint8_t _RSSI = 0;
                if (RSSI >= -50) { _RSSI = 4; }
                else if (RSSI >= -60) { _RSSI = 3; }
                else if (RSSI >= -70) { _RSSI = 2; }
                else if (RSSI >= -80) { _RSSI = 1; }
                ESP_LOGI("WI-FI", "RSSI %d", _RSSI);

                list.push_back(mOption(name.c_str(), Image(R_FILE_MANAGER_ICONS),
                                       LM_ICO_WIRELESS_0 + _RSSI)); // TODO ICONS
            }
            if (list.empty()) { return; }
            int ch = listMenu(list, count, false, LM_SETTINGS, "WI-FI");
            if (ch == -1) { return; }
            else {
                // TODO: Actually Connect to WiFi
                WifiPrompt(list.at(ch).label, enc[ch]);
                return;
            }
        }
    }
    else { ErrorWindow("You need to enable wifi before that"); }
#endif
}
