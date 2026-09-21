#include "_WiFi.h"
#include "System/Generic.h"
#include "UI/UIElements.h"
#include <System/LanguageSystem.h>
#ifndef PC
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <freertos/event_groups.h>
#include <nvs.h>
#include <atomic>
#include <cstring>

namespace {
EventGroupHandle_t wifiEvents;
esp_netif_t* station = nullptr;
esp_netif_t* hotspot = nullptr;
bool initialized = false;
std::atomic<bool> reconnect{true}, connecting{false};
std::atomic<bool> autoconnect{false};
constexpr EventBits_t connectedBit = BIT0, failedBit = BIT1;
void eventHandler(void*, esp_event_base_t base, int32_t id, void*) {
    if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        connecting = false;
        xEventGroupClearBits(wifiEvents, failedBit);
        xEventGroupSetBits(wifiEvents, connectedBit);
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(wifiEvents, connectedBit);
        xEventGroupSetBits(wifiEvents, failedBit);
        if (reconnect && !connecting) esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START && autoconnect) {
        wifi_config_t config = {};
        if (esp_wifi_get_config(WIFI_IF_STA, &config) == ESP_OK && config.sta.ssid[0]) esp_wifi_connect();
    }
}
bool check(esp_err_t err) {
    if (err == ESP_OK) return true;
    InfoWindow(NString(esp_err_to_name(err)));
    return false;
}
void saveSettings() {
    nvs_handle_t handle;
    if (nvs_open("okabe_wifi", NVS_READWRITE, &handle) != ESP_OK) return;
    nvs_set_u8(handle, "autoconnect", autoconnect);
    nvs_set_u8(handle, "reconnect", reconnect);
    nvs_commit(handle);
    nvs_close(handle);
}
bool initWifi() {
    if (initialized) return true;
    ESP_ERROR_CHECK(esp_netif_init());
    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_ERR_INVALID_STATE && !check(err)) return false;
    station = esp_netif_create_default_wifi_sta();
    hotspot = esp_netif_create_default_wifi_ap();
    wifiEvents = xEventGroupCreate();
    if (!station || !hotspot || !wifiEvents) return false;
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    if (!check(esp_wifi_init(&config))) return false;
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, eventHandler, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, eventHandler, nullptr));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_netif_set_hostname(station, HOSTNAME));
    ESP_ERROR_CHECK(esp_netif_set_hostname(hotspot, HOSTNAME));
    nvs_handle_t handle;
    if (nvs_open("okabe_wifi", NVS_READONLY, &handle) == ESP_OK) {
        uint8_t value = 0;
        if (nvs_get_u8(handle, "autoconnect", &value) == ESP_OK) autoconnect = value;
        if (nvs_get_u8(handle, "reconnect", &value) == ESP_OK) reconnect = value;
        nvs_close(handle);
    }
    initialized = true;
    return true;
}
bool setMode(wifi_mode_t mode) {
    // Suppress reconnection while deliberately stopping/reconfiguring the driver.
    connecting = true;
    esp_wifi_stop();
    xEventGroupClearBits(wifiEvents, connectedBit | failedBit);
    if (!check(esp_wifi_set_mode(mode))) { connecting = false; return false; }
    if (mode & WIFI_MODE_AP) {
        wifi_config_t ap = {};
        strcpy(reinterpret_cast<char*>(ap.ap.ssid), HOSTNAME);
        ap.ap.ssid_len = strlen(HOSTNAME);
        ap.ap.channel = 1;
        ap.ap.max_connection = 4;
        ap.ap.authmode = WIFI_AUTH_OPEN;
        if (!check(esp_wifi_set_config(WIFI_IF_AP, &ap))) { connecting = false; return false; }
    }
    bool ok = mode == WIFI_MODE_NULL || check(esp_wifi_start());
    connecting = false;
    return ok;
}
void prompt(const wifi_ap_record_t& ap) {
    NString ssid(reinterpret_cast<const char*>(ap.ssid)), password;
    std::vector<FIELD> fields = {
        FIELD(getTranslation(TextKey::WIFI_SSID), ssid, false),
        FIELD(getTranslation(TextKey::WIFI_PASSWORD), password, false)};
    if (!InputFieldS(getTranslation(TextKey::WIFI_CONNECT_TITLE), fields, LM_SETTINGS, 0,
                    getTranslation(TextKey::WIFI_CONNECT_BUTTON), getTranslation(TextKey::CANCEL_BUTTON))) return;
    wifi_config_t config = {};
    if (ssid.length() > sizeof(config.sta.ssid) || password.length() >= sizeof(config.sta.password)) {
        InfoWindow("SSID or password is too long"); return;
    }
    memcpy(config.sta.ssid, ssid.c_str(), ssid.length());
    memcpy(config.sta.password, password.c_str(), password.length());
    connecting = true;
    esp_wifi_disconnect();
    xEventGroupClearBits(wifiEvents, connectedBit | failedBit);
    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &config);
    if (err == ESP_OK) err = esp_wifi_connect();
    if (!check(err)) { connecting = false; return; }
    InfoWindow(getTranslation(TextKey::IW_WIFI_CONNECTING), IW_TITLE::INFO);
    // Wait for DHCP completion, not just radio association.
    auto bits = xEventGroupWaitBits(wifiEvents, connectedBit, pdFALSE, pdFALSE, pdMS_TO_TICKS(10000));
    connecting = false;
    InfoWindow(getTranslation(bits & connectedBit ? TextKey::IW_WIFI_CONNECTED : TextKey::IW_WIFI_ERR_CONN_FAIL));
    drawStatusBar(true);
}
void scan() {
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if (!(mode & WIFI_MODE_STA)) { InfoWindow(getTranslation(TextKey::IW_WIFI_EN_NEEDED)); return; }
    InfoWindow(getTranslation(TextKey::IW_WIFI_SCANNING), IW_TITLE::INFO);
    if (!check(esp_wifi_scan_start(nullptr, true))) return;
    uint16_t count = 0;
    esp_wifi_scan_get_ap_num(&count);
    std::vector<wifi_ap_record_t> records(count);
    if (!count) { esp_wifi_clear_ap_list(); return; }
    if (!check(esp_wifi_scan_get_ap_records(&count, records.data()))) return;
    std::vector<mOption> options;
    for (const auto& ap : records) {
        int strength = ap.rssi >= -50 ? 4 : ap.rssi >= -60 ? 3 : ap.rssi >= -70 ? 2 : ap.rssi >= -80 ? 1 : 0;
        options.emplace_back(reinterpret_cast<const char*>(ap.ssid), Image(R_FILE_MANAGER_ICONS), LM_ICO_WIRELESS_0 + strength);
    }
    int choice = listMenu(options, options.size(), false, LM_SETTINGS, getTranslation(TextKey::LM_WIFI));
    if (choice >= 0 && choice < count) prompt(records[choice]);
}
void wifiSettings() {
    for (;;) {
        std::vector<mOption> options = {
            mOption(getTranslation(TextKey::WIFI_TOGGLE_AUTOCONNECT), Image(R_FILE_MANAGER_ICONS), autoconnect ? LM_ICO_CHECK_CHECKED : LM_ICO_CHECK_UNCHECKED),
            mOption(getTranslation(TextKey::WIFI_TOGGLE_AUTORECONNECT), Image(R_FILE_MANAGER_ICONS), reconnect ? LM_ICO_CHECK_CHECKED : LM_ICO_CHECK_UNCHECKED),
            mOption(getTranslation(TextKey::WIFI_SET_HOSTNAME))};
        int choice = listMenu(options, options.size(), false, LM_SETTINGS, getTranslation(TextKey::LM_WIFI_SETTINGS));
        if (choice < 0) return;
        if (choice == 0) { autoconnect = !autoconnect; saveSettings(); }
        if (choice == 1) { reconnect = !reconnect; saveSettings(); }
        if (choice == 2) {
            const char* value = nullptr;
            esp_netif_get_hostname(station, &value);
            NString name(value);
            std::vector<FIELD> fields = {FIELD(getTranslation(TextKey::WIFI_HOSTNAME_FIELD), name, false)};
            if (InputFieldS(getTranslation(TextKey::WIFI_SET_HOSTNAME), fields)) check(esp_netif_set_hostname(station, name.c_str()));
        }
    }
}
}
#endif
void initializeWiFi() {
#ifndef PC
    // Avoid allocating the Wi-Fi stack on boot when auto-connect is disabled.
    nvs_handle_t handle;
    uint8_t enabled = 0;
    if (nvs_open("okabe_wifi", NVS_READONLY, &handle) == ESP_OK) {
        nvs_get_u8(handle, "autoconnect", &enabled);
        nvs_close(handle);
    }
    if (enabled && initWifi()) setMode(WIFI_MODE_STA);
#endif
}
void WiFiMenu() {
#ifndef PC
    if (!initWifi()) return;
    for (;;) {
        wifi_mode_t mode = WIFI_MODE_NULL;
        esp_wifi_get_mode(&mode);
        std::vector<mOption> options = {
            mOption(getTranslation(TextKey::LM_WIFI), Image(R_FILE_MANAGER_ICONS), mode & WIFI_MODE_STA ? LM_ICO_CHECK_CHECKED : LM_ICO_CHECK_UNCHECKED),
            mOption(getTranslation(TextKey::WIFI_HOTSPOT_TOGGLE), Image(R_FILE_MANAGER_ICONS), mode & WIFI_MODE_AP ? LM_ICO_CHECK_CHECKED : LM_ICO_CHECK_UNCHECKED),
            mOption(getTranslation(TextKey::LM_WIFI_SETTINGS)),
            mOption(getTranslation(TextKey::WIFI_SCAN_BUTTON))};
        int choice = listMenu(options, options.size(), false, LM_SETTINGS, getTranslation(TextKey::LM_WIFI));
        if (choice < 0) return;
        if (choice == 0) setMode(static_cast<wifi_mode_t>(mode ^ WIFI_MODE_STA));
        if (choice == 1) setMode(static_cast<wifi_mode_t>(mode ^ WIFI_MODE_AP));
        if (choice == 2) wifiSettings();
        if (choice == 3) scan();
        drawStatusBar(true);
    }
#else
    InfoWindow(getTranslation(TextKey::IW_NOT_SUPPORTED));
#endif
}
