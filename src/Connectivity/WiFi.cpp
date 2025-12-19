#include "../System/Generic.h"
#include "../UI/UIElements.h"
#include "_WiFi.h"

#ifdef WiFi_h
void WifiPrompt(NString ssid, uint8_t encryptionType, NString password = NString()) {
    int pos       = 0;
    int textboxes = 2;
    int buttons   = 2;
    int direction;

    res.DrawImage(R_LIST_MENU_BACKGROUND);
    res.DrawImage(R_LIST_HEADER_BACKGROUND);
    res.DrawImage(R_LIST_HEADER_ICONS, LM_SETTINGS);
    drawStatusBar();
    tft.setCursor(30, 45);
    tft.setTextSize(1);
    changeFont(1);
    tft.setTextColor(0xffff);
    NString boxString[textboxes] = {ssid, password};
    tft.print("Connect to WiFi");
    InputField("SSID", ssid, 70, true, false, false);
    if (encryptionType != WIFI_AUTH_OPEN) {
        InputField("Password", password, 120, true, false, false);
    }
    button("Connect", 10, 285, 100, 28);
    button("Cancel", 120, 285, 100, 28);
    while (true) {
        switch (pos) {
        case 0:
            boxString[pos] =
                InputField("SSID", boxString[pos], 70, false, false, true, &direction);
            break;
        case 1:
            if (encryptionType != WIFI_AUTH_OPEN) {
                boxString[pos] =
                    InputField("Password", boxString[pos], 120, false, false, true, &direction);
            }
            else if (direction == UP) { pos--; }
            else if (direction == DOWN) { pos++; }
            break;
        case 2:
            direction = 0;
            button("Connect", 10, 285, 100, 28, true, &direction);
            if (direction == SELECT) {
                if (boxString[0].isEmpty() || boxString[1].isEmpty()) { break; }
                if (encryptionType == WIFI_AUTH_OPEN) { WiFi.begin(boxString[0].c_str()); }
                else { WiFi.begin(boxString[0].c_str(), boxString[1].c_str()); }
                return;
            }

            break;
        case 3:
            direction = 0;
            button("Cancel", 120, 285, 100, 28, true, &direction);
            if (direction == SELECT) { return; }
            break;
        default: pos = 0; break;
        }
        switch (direction) {
        case DOWN:
            if (pos < textboxes) { pos++; }
            else if (pos < textboxes + buttons)
                ;
            else { pos = 0; }
            break;
        case UP:
            if (pos > textboxes) { pos = textboxes - 1; }

            else if (pos > 0) { pos--; }

            break;
        default: break;
        case RIGHT:
            if (pos < textboxes + buttons - 1) { pos++; }
            break;
        case LEFT:
            if (pos > textboxes) { pos--; }
            break;
        }
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
