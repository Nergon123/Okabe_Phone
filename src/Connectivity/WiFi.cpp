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
        InfoWindow("Connecting...", "INFO", false, TFT_BLUE);
        if ((WiFiGenericClass::getMode() & WIFI_MODE_STA) == 0) {
            InfoWindow("Wifi not enabled");
            return;
        }
        unsigned long start = millis();
        while (
            ((!WiFi.status() || WiFi.status() >= WL_DISCONNECTED) && (millis() - start) < 10000)) {
            if (buttonsHelding() != -1) { break; }
            delay(100);
        }
        wl_status_t result = WiFi.status();
        drawStatusBar(true);
        NString errorResult;
        bool    isError = result != WL_CONNECTED;
        switch (result) {
        case WL_IDLE_STATUS: errorResult = "Idle (maybe timeout)"; break;
        case WL_NO_SSID_AVAIL: errorResult = "No SSID avaliable"; break;
        case WL_SCAN_COMPLETED:
            errorResult = "Scan Completed\n(this one shouldn't be here)";
            break;
        case WL_CONNECTED: errorResult = "Connected!"; break;
        case WL_CONNECT_FAILED: errorResult = "Connection Failed"; break;
        case WL_CONNECTION_LOST: errorResult = "Connection Lost"; break;
        case WL_DISCONNECTED: errorResult = "Disconnected\n(maybe timeout)"; break;
        case WL_NO_SHIELD: errorResult = "No WiFi shield"; break;
        default: errorResult = "Unknown Status(" + NString(result) + ").";
        }
        InfoWindow(errorResult, isError ? "ERROR" : "INFO", true, isError ? TFT_RED : TFT_BLUE);
    }
}
#endif
// List available WiFi networks
void WiFiList() {
#ifdef WiFi_h
    if (WiFi.getMode() == WIFI_MODE_STA || WiFi.getMode() == WIFI_MODE_APSTA) {
        while (true) {
            InfoWindow("Scanning for WiFi Networks...", "INFO", false, TFT_BLUE);
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

                list.push_back(
                    mOption(name.c_str(), Image(R_FILE_MANAGER_ICONS), LM_ICO_WIRELESS_0 + _RSSI));
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
    else { InfoWindow("You need to enable wifi before that"); }
#endif
}

#define WIFI_BIT_AP  0b01
#define WIFI_BIT_STA 0b10
#ifdef WiFi_h
static wifi_mode_t bitsToWifiMode(uint8_t bits) {
    if ((bits & WIFI_BIT_AP) && (bits & WIFI_BIT_STA)) { return WIFI_MODE_APSTA; }
    if (bits & WIFI_BIT_AP) { return WIFI_MODE_AP; }
    if (bits & WIFI_BIT_STA) { return WIFI_MODE_STA; }
    return WIFI_MODE_NULL;
}

void setHostname(bool Ap) {
    NString            hostname = WiFi.getHostname();
    std::vector<FIELD> fields   = {FIELD("Hostname", hostname, false)};
    if (InputFieldS("Set Hostname", fields)) {
        if (Ap) { WiFi.softAPsetHostname(hostname.c_str()); }
        else { WiFi.setHostname(hostname.c_str()); };
    }
}
void WiFiSettings() {
    std::vector<mOption> options = {
        mOption("Auto Connect", Image(R_FILE_MANAGER_ICONS, LM_ICO_CHECK_UNCHECKED)),
        mOption("Auto Reconnect", Image(R_FILE_MANAGER_ICONS, LM_ICO_CHECK_UNCHECKED)),
        mOption("Set Hostname")};
    int choice = LISTMENU_NULL;
    while (choice != LISTMENU_EXIT) {
        options.at(0).icon_index =
            WiFi.getAutoConnect() ? LM_ICO_CHECK_CHECKED : LM_ICO_CHECK_UNCHECKED;
        options.at(1).icon_index =
            WiFi.getAutoReconnect() ? LM_ICO_CHECK_CHECKED : LM_ICO_CHECK_UNCHECKED;
        choice = listMenu(options, options.size(), false, LM_SETTINGS, "Wi-Fi Settings");
        switch (choice) {
        case 0: WiFi.setAutoConnect(!WiFi.getAutoConnect()); break;
        case 1: WiFi.setAutoReconnect(!WiFi.getAutoReconnect()); break;
        // SetHostname TODO Implement
        case 2: setHostname(false); break;
        }
    }
}
void HotspotSettings() {
    return; /////////////
    std::vector<mOption> options = {
        mOption("Change Properties"),
    };

    int choice = LISTMENU_NULL;
    while (choice != LISTMENU_EXIT) {

        listMenu(options, options.size(), false, LM_SETTINGS, "Wi-Fi Settings", false, choice);
        switch (choice) {
        case 0: break;
        }
    }
}
#endif

void WiFiMenu() {
#ifdef WiFi_h
    std::vector<mOption> options = {
        mOption("Wi-Fi", Image(R_FILE_MANAGER_ICONS, LM_ICO_CHECK_UNCHECKED)),
        mOption("Wi-Fi Hotspot", Image(R_FILE_MANAGER_ICONS, LM_ICO_CHECK_UNCHECKED)),
        mOption("Wi-Fi Settings"), mOption("Hotspot settings"), mOption("Scan WiFi networks")};
    int choice = 0;
    while (choice != LISTMENU_EXIT) {
        uint8_t wifimode;

        switch (WiFi.getMode()) {
        case WIFI_MODE_NULL: wifimode = 0; break;
        case WIFI_MODE_AP: wifimode = WIFI_BIT_AP; break;
        case WIFI_MODE_STA: wifimode = WIFI_BIT_STA; break;
        case WIFI_MODE_APSTA: wifimode = WIFI_BIT_AP | WIFI_BIT_STA; break;
        default: wifimode = 0; break;
        }
        options.at(1).icon_index =
            wifimode & WIFI_BIT_AP ? LM_ICO_CHECK_CHECKED : LM_ICO_CHECK_UNCHECKED;
        options.at(0).icon_index =
            wifimode & WIFI_BIT_STA ? LM_ICO_CHECK_CHECKED : LM_ICO_CHECK_UNCHECKED;
        choice = listMenu(options, options.size(), false, LM_SETTINGS, "Wi-Fi", 1, choice);
        switch (choice) {
        case 0: // toggle Wi-Fi
            wifimode ^= WIFI_BIT_STA;
            WiFi.mode(bitsToWifiMode(wifimode));
            drawStatusBar(true);
            break;

        case 1: // toggle Hotspot
            wifimode ^= WIFI_BIT_AP;
            WiFi.mode(bitsToWifiMode(wifimode));
            drawStatusBar(true);
            break;
        case 2: WiFiSettings(); break;
        case 3: HotspotSettings(); break;
        case 4: WiFiList(); break;
        }
    }
#else
    InfoWindow("Not supported");

#endif
}
