#pragma once
#include "Platform/Hardware/Hardware.h"
#ifndef PC
#include "Platform/Hardware/Drivers/Battery/IP5306.h"
#include <Connectivity/SIM.h>
#include <Esp.h>
#include <HTTPClient.h>
#include <MCP23017.h>
#include <Platform/FileSystem/ESP.h>
#include <Platform/FileSystem/VFS.h>
#include <Platform/Graphics/TFTESPIRenderTarget.h>
#include <SD.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <UI/UIElements.h>
#include <esp_crc.h>
#include <esp_debug_helpers.h>
#include <esp_task_wdt.h>
#include <esp_wifi.h>
#define FAST_SD_FREQ          20 * 1000 * 1000
#define SAFE_SD_FREQ          1 * 1000 * 1000
#define SERIAL_BAUD_RATE      115200
#define FAST_SERIAL_BAUD_RATE 921600
#define SIM_BAUD_RATE         115200
#define SIM_INT_PIN           38

#define MCP23017_ADDR 0x20

#define SD_CS   13
#define SD_SCK  14
#define SD_MISO 2
#define SD_MOSI 15

#define SIM_RX_PIN 35
#define SIM_TX_PIN 26
#define I2C_SDA    21
#define I2C_SCL    22

#define SimSerial Serial1

MCP23017 mcp(MCP23017_ADDR);

class DEV_ESP32 : public iHW {
  public:
    void init() override {
        esp_task_wdt_deinit();
        esp_task_wdt_init(10000, false);
        esp_log_level_set("ledc", ESP_LOG_NONE); // brightness logger

        Wire.setPins(I2C_SDA, I2C_SCL);
        if (Wire.begin()) {
            ESP_LOGI("I2C", "I2C initalized at SDA:%d SCL:%d Freq: %d kHz", I2C_SDA, I2C_SCL,
                     Wire.getClock() / 1000);
        }
        else {
            ESP_LOGE("I2C", "Error occured when initializing I2C (SDA:%d SCL:%d Freq: %d kHz)",
                     I2C_SDA, I2C_SCL, Wire.getClock() / 1000);
        }
        Serial.begin(SERIAL_BAUD_RATE);
        ESP_LOGI("SERIAL", "Serial initalized at %d baud", SERIAL_BAUD_RATE);
        SimSerial.begin(SIM_BAUD_RATE, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
        ESP_LOGI("SIM_CARD_SERIAL", "Sim Card Serial initialized at %d", SIM_BAUD_RATE);
        initMCP();

        pinMode(TFT_BL, OUTPUT);
        setScreenBrightness(0);
        attachInterrupt(SIM_INT_PIN, simInterrupt, RISING);
        keypad_exists = checkI2Cdevices(MCP23017_ADDR);
        if (keypad_exists) { ESP_LOGI("KEYPAD", "MCP23017 Initalized"); }
        else { ESP_LOGE("KEYPAD", "MCP23017 cannot be initalized"); }
        charger_exists = checkI2Cdevices(IP5306_ADDR);
    };

    void initStorage() override {
        SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
        ESP_LOGI("SD", "SPI started");
        bootText("Initializing SDCard...");
        sdcard_exists       = false; // initSDcard(true);
        IFileSystem* spiffs = new Esp32FileSystem(&SPIFFS, FS_INTERNAL);
        IFileSystem* sdcard = new Esp32FileSystem(&SD, FS_EXTERNAL);
        if (sdcard_exists) { VFS.mount("/sd", sdcard); }
        bootText("Initializing SPIFFS...");
        if (SPIFFS.begin()) { VFS.mount("/spiffs", spiffs); }
    }
    void  postScreenInit() override { showResetReason(); }
    ulong micros() override { return ::micros(); };
    void  delay(ulong ms) override { ::delay(ms); };
    void  setCPUSpeed(CPU_SPEED speed) override {
        // setCpuFrequencyMhz(FAST_CPU_FREQ_MHZ);
    };
    CPU_SPEED   getCPUSpeed() override { return CPU_DEFAULT; };
    const char* getDeviceName() override { return ESP.getChipModel(); };
    void        shutdown() override { reboot(); };
    void        reboot() override { ESP.restart(); };

    void setScreenBrightness(int8_t value) override { analogWrite(TFT_BL, (value * 255) / 100); };
    char getCharInput() override {
        if (Serial.available()) { return Serial.read(); }
        return 0;
    };
    int getWifiStrength() override {
        if (WiFi.getMode() == WIFI_MODE_NULL) { return -1; }
        if (WiFi.status() != WL_CONNECTED) { return 0; }
        int8_t rssi = WiFi.RSSI();
        if (rssi >= -50) { return 4; }
        if (rssi >= -60) { return 3; }
        if (rssi >= -70) { return 2; }
        return 1;
    };
    int getKeyInput() override {
        if (!keypad_exists) { return 0; }
        mcp.portMode(MCP23017Port::A, 0xFF);
        mcp.portMode(MCP23017Port::B, 0);
        mcp.writePort(MCP23017Port::B, 0x00);
        uint8_t a = mcp.readPort(MCP23017Port::A);

        mcp.portMode(MCP23017Port::B, 0xFF);
        mcp.portMode(MCP23017Port::A, 0);
        mcp.writePort(MCP23017Port::A, 0x00);
        uint8_t b = mcp.readPort(MCP23017Port::B);

        a = ~a;
        a >>= 1;
        b = ~b;

        uint8_t ar = 0xFF, br = 0xFF;
        for (int i = 0; i < 8; ++i) {
            if (a & (1 << i) && ar == 0xFF) { ar = i; }
            if (b & (1 << i) && br == 0xFF) { br = i; }
        }
        ar++;
        br++;

        if (ar != 0xFF && br != 0xFF) {
            uint8_t res = ar == 0 && br == 0 ? 0 : 21 - (ar * 3) + br;
            return res;
        }
        return 0;
    };
    int getBatteryCharge() override {
        if (charger_exists) { return bat.getBatteryLevel(); }
        return 3;
    };
    bool isCharging() override {
        if (charger_exists) { return bat.isChargerConnected(); }
        return false;
    };
    void updateFrequencies() override {

    };
    RenderTarget* GetScreen() override { return setupTFTESPIRenderTarget(); }
    HttpAnswer    httpSend(HttpMethod method, const NString& url, NString& payload,
                           const std::vector<HttpHeader>& headers, uint16_t timeout) override {
        HttpAnswer answ;
        http.setTimeout(timeout);
        if (!http.begin(url.c_str())) { return answ; }
        for (const HttpHeader& header : headers) {
            http.addHeader(header.name.c_str(), header.content.c_str());
        }

        switch (method) {
        case HttpMethod::GET: answ.code = http.GET(); break;
        case HttpMethod::POST: answ.code = http.POST(payload.c_str()); break;
        case HttpMethod::PUT: answ.code = http.PUT(payload.c_str()); break;
        case HttpMethod::DELETE_: answ.code = http.sendRequest("DELETE", payload.c_str()); break;
        case HttpMethod::PATCH: answ.code = http.sendRequest("PATCH", payload.c_str()); break;
        }

        if (answ.code > 0) { answ.response = http.getString().c_str(); }
        else { answ.response = "Error: " + NString(http.errorToString(answ.code).c_str()); }
        http.end();
        return answ;
    }
    void downloadFile(NString& url, IFile* fileToDownload,
                      std::function<void(size_t, size_t)> progressCallback) {
        HTTPClient http;
        http.begin(url.c_str());
        int    httpCode = http.GET();
        size_t fileSize = http.getSize();
        if (httpCode == HTTP_CODE_OK) {
            WiFiClient* stream = http.getStreamPtr();

            if (!fileToDownload->available()) {
                InfoWindow("Failed to open file for writing");
                return;
            }

            uint8_t buf[512];
            size_t  downloaded = 0;
            int     len        = 0;
            while ((len = stream->readBytes(buf, sizeof(buf))) > 0) {
                fileToDownload->write(buf, len);
                downloaded += len;
                if (progressCallback) { progressCallback(downloaded, fileSize); }
            }
            fileToDownload->close();
            InfoWindow("File downloaded!");
        }
        else { InfoWindow(NString::format("HTTP error: %d\n", httpCode)); }
        http.end();
    }
    uint32_t crc32(uint32_t crc, const uint8_t* buf, size_t len) override {
        return esp_crc32_le(crc, buf, len);
    }

  private:
    HTTPClient http;

    void showResetReason() {
        return;
        const char* reason;
        switch (esp_reset_reason()) {
        case ESP_RST_PANIC: reason = "CORE PANIC"; break;
        case ESP_RST_INT_WDT: reason = "INTERRUPT WATCHDOG TIMEOUT"; break;
        case ESP_RST_TASK_WDT: reason = "TASK WATCHDOG TIMEOUT"; break;
        case ESP_RST_WDT: reason = "WATCHDOG TIMEOUT"; break;
        // case ESP_RST_DEEPSLEEP: break;
        case ESP_RST_BROWNOUT: reason = "BROWNOUT"; break;
        // case ESP_RST_SDIO: break;
        default: return;
        }
        for (int x = 0; x < 240; x++) {
            for (int y = 0; y < 320; y++) {
                if (x % 4 == 1 && y % 4 == 1) { tft.drawPixel(x, y, rand() % 0xAAAA); }
            }
        }
        tft.setCursor(0, 20);
        tft.setTextSize(4);
        tft.setTextColor(0xffff, 0, true);
        tft.println("> FATAL! <");
        tft.setTextSize(1);
        tft.printf("\n\n%s\n\nPress anything to continue.", reason);
        currentRenderTarget->present();
        ulong old_millis = hw->millis();
        while ((!getKeyInput() && !getCharInput())) {
            delay(50);
            if (!NI_delay(old_millis, 5000)) { break; }
        };
        tft.fillScreen(0);
    }

    void initMCP() {
        mcp.writeRegister(MCP23017Register::GPIO_A, 0x00); // Reset port A
        mcp.writeRegister(MCP23017Register::GPIO_B, 0x00); // Reset port B
    }

    bool initSDcard(bool fast) {
        uint32_t freq  = fast ? FAST_SD_FREQ : SAFE_SD_FREQ;
        int      tries = 0;

        if (!SD.begin(SD_CS, SPI, SAFE_SD_FREQ)) { return false; }

        while (tries < 5) {
            if (freq >= getCpuFrequencyMhz() * 1000000) { freq /= 4; }
            ESP_LOGI("SD", "TRYING %u Hz", freq);

            if (SD.begin(SD_CS, SPI, freq)) {
                IFileSystem* sd = new Esp32FileSystem(&SD, FS_EXTERNAL);
                VFS.mount("/sd", sd);
                return true;
            }
            freq /= 2;
            tries++;
        }
        return false; // failed to init
    }
    bool sdcard_exists  = false;
    bool keypad_exists  = false;
    bool charger_exists = false;
    bool checkI2Cdevices(byte device) {
        Wire.beginTransmission(device);
        bool error = Wire.endTransmission() != 0;
        return !error;
    }
};
#endif