#ifndef PC
#include "ESP32.h"
#include <Connectivity/_WiFi.h>
#include <GlobalVariables.h>
#include <Platform/Audio/I2S/I2SAudio.h>
#include <Platform/Hardware/Drivers/Battery/IP5306.h>
#include <System/Generic.h>
#include <System/LanguageSystem.h>
#include <UI/UIElements.h>
#include <driver/sdspi_host.h>
#include <driver/spi_master.h>
#include <esp_crc.h>
#include <esp_crt_bundle.h>
#include <esp_http_client.h>
#include <esp_spiffs.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <esp_vfs_fat.h>
#include <esp_wifi.h>
#include <fcntl.h>
#include <memory>
#include <nvs_flash.h>
#include <op_keypad.h>
#include <op_peripherals.h>
#include <sdmmc_cmd.h>
#include <sys/time.h>
#include <unistd.h>

void DEV_ESP32::init() {
    esp_err_t err = nvs_flash_init();
    // Preserve existing NVS data on errors; recovery must be explicit.
    ESP_ERROR_CHECK(err);
    initializeWiFi();
    ESP_LOGI("HW", "Initializing I2C on SDA=%d SCL=%d", I2C_SDA, I2C_SCL);
    err = okabe::initI2C();
    if (err != ESP_OK) { ESP_LOGW("I2C", "Unavailable: %s", esp_err_to_name(err)); }
    initKeypad();
    charger = okabe::probeI2C(IP5306_ADDR);
    err     = okabe::initModem();
    if (err != ESP_OK) { ESP_LOGW("MODEM", "Unavailable: %s", esp_err_to_name(err)); }
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags >= 0) { fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK); }
    ESP_ERROR_CHECK(okabe::initBacklight());
    ESP_LOGI("HW", "Peripheral initialization complete");
#ifdef HAS_I2S
    audioSource = new I2SAudio();
#else
    audioSource = new NullAudio();
#endif
}
void DEV_ESP32::postScreenInit() { setScreenBrightness(currentBrightness); }
void DEV_ESP32::initStorage() {
    bootText(getTranslation(TextKey::BOOT_INIT_SPIFFS));
    esp_vfs_spiffs_conf_t conf  = {};
    conf.base_path              = "/spiffs";
    conf.max_files              = 8;
    conf.format_if_mount_failed = false;
    esp_err_t err               = esp_vfs_spiffs_register(&conf);
    if (err == ESP_OK) { VFS.mount("/spiffs", new Std2FileSystem("/spiffs", FS_INTERNAL)); }
    else { ESP_LOGE("FS", "SPIFFS mount failed: %s", esp_err_to_name(err)); }
    if (SD_CS < 0 || SD_SCK < 0 || SD_MOSI < 0 || SD_MISO < 0) { return; }
    bootText(getTranslation(TextKey::BOOT_INIT_SDCARD));
    spi_bus_config_t bus = {};
    bus.mosi_io_num      = SD_MOSI;
    bus.miso_io_num      = SD_MISO;
    bus.sclk_io_num      = SD_SCK;
    bus.quadwp_io_num    = -1;
    bus.quadhd_io_num    = -1;
    sdmmc_host_t host    = SDSPI_HOST_DEFAULT();
    host.slot            = SPI3_HOST; // LCD owns SPI2.
    err                  = spi_bus_initialize(SPI3_HOST, &bus, SPI_DMA_CH_AUTO);
    if (err != ESP_OK) {
        ESP_LOGE("SD", "Bus init failed: %s", esp_err_to_name(err));
        return;
    }
    sdspi_device_config_t slot             = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot.host_id                           = SPI3_HOST;
    slot.gpio_cs                           = static_cast<gpio_num_t>(SD_CS);
    esp_vfs_fat_sdmmc_mount_config_t mount = {};
    mount.max_files                        = 8;
    mount.allocation_unit_size             = 16 * 1024;
    sdmmc_card_t* card                     = nullptr;
    err = esp_vfs_fat_sdspi_mount("/sd", &host, &slot, &mount, &card);
    if (err == ESP_OK) { VFS.mount("/sd", new Std2FileSystem("/sd", FS_EXTERNAL)); }
    else {
        ESP_LOGW("SD", "Mount failed: %s", esp_err_to_name(err));
        spi_bus_free(SPI3_HOST);
    }
}
ulong DEV_ESP32::micros() { return static_cast<ulong>(esp_timer_get_time()); }
ulong DEV_ESP32::millis() { return static_cast<ulong>(esp_timer_get_time() / 1000); }
void  DEV_ESP32::delay(ulong ms) { vTaskDelay(std::max<TickType_t>(1, pdMS_TO_TICKS(ms))); }
void  DEV_ESP32::timeSet(time_t t) {
    struct timeval now = {t, 0};
    settimeofday(&now, nullptr);
}
time_t      DEV_ESP32::timeGet() { return time(nullptr); }
const char* DEV_ESP32::getDeviceName() { return CONFIG_IDF_TARGET; }
void        DEV_ESP32::shutdown() { reboot(); }
void        DEV_ESP32::reboot() { esp_restart(); }
void DEV_ESP32::setScreenBrightness(int8_t value) { okabe::setBacklight(std::max(0, int(value))); }
char DEV_ESP32::getCharInput() {
    char c;
    return read(STDIN_FILENO, &c, 1) == 1 ? c : 0;
}
int DEV_ESP32::getWifiStrength() {
    wifi_mode_t mode;
    if (esp_wifi_get_mode(&mode) != ESP_OK || !(mode & WIFI_MODE_STA)) { return -1; }
    wifi_ap_record_t ap = {};
    if (esp_wifi_sta_get_ap_info(&ap) != ESP_OK) { return 0; }
    return ap.rssi >= -50 ? 4 : ap.rssi >= -60 ? 3 : ap.rssi >= -70 ? 2 : 1;
}
void DEV_ESP32::initKeypad() {
    keypad = false;
    // Address straps select 0x20..0x27; accept a single responding expander.
    uint8_t detected = 0;
    for (uint8_t address = 0x20; address <= 0x27; ++address) {
        if (okabe::probeI2C(address)) {
            keypadAddress = address;
            ++detected;
        }
    }
    if (detected == 1) {
        esp_err_t err = okabe::initKeypad(keypadAddress);
        keypad        = err == ESP_OK;
        if (keypad) {
            ESP_LOGI("KEYPAD", "MCP23017 ready at 0x%02x (SDA=%d SCL=%d)", keypadAddress, I2C_SDA,
                     I2C_SCL);
        }
        else {
            ESP_LOGW("KEYPAD", "MCP23017 initialization at 0x%02x failed: %s", keypadAddress,
                     esp_err_to_name(err));
        }
    }
    else if (detected == 0) {
        ESP_LOGW("KEYPAD", "No MCP23017 ACK at 0x20..0x27 (SDA=%d SCL=%d); retrying in 5s",
                 I2C_SDA, I2C_SCL);
    }
    else { ESP_LOGW("KEYPAD", "Multiple expanders found; keypad address is ambiguous"); }
    keypadRetryAt = millis();
}
int DEV_ESP32::getKeyInput() {
    if (!keypad) {
        if (millis() - keypadRetryAt >= 5000) { initKeypad(); }
        if (!keypad) { return 0; }
    }
    int       key = 0;
    esp_err_t err = okabe::scanKeypad(keypadAddress, key);
    if (err != ESP_OK) {
        ESP_LOGW("KEYPAD", "Scan at 0x%02x failed: %s; reinitializing in 5s", keypadAddress,
                 esp_err_to_name(err));
        keypad        = false;
        keypadRetryAt = millis();
        return 0;
    }
    return key;
}
int           DEV_ESP32::getBatteryCharge() { return charger ? bat.getBatteryLevel() : 3; }
bool          DEV_ESP32::isCharging() { return charger && bat.isChargerConnected(); }
RenderTarget* DEV_ESP32::GetScreen() { return setupIDFRenderTarget(); }
uint32_t      DEV_ESP32::crc32(uint32_t crc, const uint8_t* buf, size_t len) {
    return esp_crc32_le(crc, buf, len);
}

namespace {
struct ClientDeleter {
    void operator()(esp_http_client* p) const { esp_http_client_cleanup(p); }
};
using Client = std::unique_ptr<esp_http_client, ClientDeleter>;
} // namespace
HttpAnswer DEV_ESP32::httpSend(HttpMethod method, const NString& url, NString& payload,
                               const std::vector<HttpHeader>& headers, uint16_t timeout) {
    std::string              response;
    esp_http_client_config_t config = {};
    config.url                      = url.c_str();
    config.timeout_ms               = timeout ? timeout : 10000;
    config.crt_bundle_attach        = esp_crt_bundle_attach;
    config.user_data                = &response;
    config.event_handler            = [](esp_http_client_event_t* event) -> esp_err_t {
        if (event->event_id == HTTP_EVENT_ON_DATA) {
            static_cast<std::string*>(event->user_data)
                ->append(static_cast<char*>(event->data), event->data_len);
        }
        return ESP_OK;
    };
    Client client(esp_http_client_init(&config));
    if (!client) { return {-1, "HTTP client allocation failed"}; }
    const esp_http_client_method_t methods[] = {HTTP_METHOD_GET, HTTP_METHOD_POST, HTTP_METHOD_PUT,
                                                HTTP_METHOD_DELETE, HTTP_METHOD_PATCH};
    esp_http_client_set_method(client.get(), methods[static_cast<unsigned>(method)]);
    for (const auto& header : headers) {
        esp_http_client_set_header(client.get(), header.name.c_str(), header.content.c_str());
    }
    if (!payload.isEmpty()) {
        esp_http_client_set_post_field(client.get(), payload.c_str(), payload.length());
    }
    esp_err_t err = esp_http_client_perform(client.get());
    if (err != ESP_OK) { return {-1, esp_err_to_name(err)}; }
    return {esp_http_client_get_status_code(client.get()), NString(response)};
}
void DEV_ESP32::downloadFile(NString& url, IFile* file,
                             std::function<void(size_t, size_t)> progress) {
    if (!file) {
        InfoWindow(getTranslation(TextKey::IW_FILE_OPEN_FAILED));
        return;
    }
    esp_http_client_config_t config = {};
    config.url                      = url.c_str();
    config.timeout_ms               = 15000;
    config.crt_bundle_attach        = esp_crt_bundle_attach;
    Client client(esp_http_client_init(&config));
    bool   success = false;
    if (client && esp_http_client_open(client.get(), 0) == ESP_OK) {
        int64_t length = esp_http_client_fetch_headers(client.get());
        if (length >= 0 && esp_http_client_get_status_code(client.get()) == 200) {
            char   buffer[1024];
            size_t downloaded = 0;
            for (;;) {
                int got = esp_http_client_read(client.get(), buffer, sizeof(buffer));
                if (got < 0) { break; }
                if (!got) {
                    success = esp_http_client_is_complete_data_received(client.get());
                    break;
                }
                if (file->write(buffer, got) != static_cast<size_t>(got)) { break; }
                downloaded += got;
                if (progress) { progress(downloaded, length); }
            }
        }
    }
    file->close();
    InfoWindow(success ? getTranslation(TextKey::IW_FILE_DOWNLOADED) : NString("Download failed"));
}
#endif
