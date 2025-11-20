#pragma once
#include "../Hardware.h"
#ifndef PC
#include <Esp.h>
#include "../Drivers/Battery/IP5306.h"
#include <MCP23017.h>
#include <Platform/FileSystem/ESP.h>
#include <Platform/FileSystem/VFS.h>
#include <Platform/Graphics/TFTESPIRenderTarget.h>
#include <SD.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <esp_task_wdt.h>

#define FAST_SD_FREQ          20 * 1000 * 1000
#define SAFE_SD_FREQ          1 * 1000 * 1000
#define SERIAL_BAUD_RATE      115200
#define FAST_SERIAL_BAUD_RATE 921600
#define SIM_BAUD_RATE         115200

#define MCP23017_ADDR 0x20

#define SD_CS   13
#define SD_SCK  14
#define SD_MISO 2
#define SD_MOSI 15

#define SIM_RX_PIN 35
#define SIM_TX_PIN 26

#define SimSerial Serial1

MCP23017 mcp(MCP23017_ADDR);

class DEV_ESP32 : public iHW {
  public:
    void init() override {
        esp_task_wdt_deinit();
        esp_task_wdt_init(10000, false);

        Serial.begin(SERIAL_BAUD_RATE);
        ESP_LOGI("SERIAL", "Serial initalized at %d baud", SERIAL_BAUD_RATE);
        SimSerial.begin(SIM_BAUD_RATE, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
        ESP_LOGI("SIM_CARD_SERIAL", "Sim Card Serial initialized at %d", SIM_BAUD_RATE);
        initMCP();

        pinMode(TFT_BL, OUTPUT);
        analogWrite(TFT_BL, 0);

        keypad_exists  = checkI2Cdevices(MCP23017_ADDR);
        charger_exists = checkI2Cdevices(IP5306_ADDR);
    };
    void initStorage() override {
        SPIFFS.begin();
        SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
        ESP_LOGI("SD", "SPI started");
        initSDcard(true);
        IFileSystem* spiffs = new Esp32FileSystem(&SPIFFS, FS_INTERNAL);
        IFileSystem* sdcard = new Esp32FileSystem(&SD, FS_EXTERNAL);
        sdcard->begin();
        VFS.mount("/sd", sdcard);
        spiffs->begin();
        VFS.mount("/spiffs", spiffs);
    }
    ulong micros() override { return ::micros(); };

    void setCPUSpeed(CPU_SPEED speed) override {
        // setCpuFrequencyMhz(FAST_CPU_FREQ_MHZ);
    };
    CPU_SPEED getCPUSpeed() override {

    };
    const char* getDeviceName() override { return ESP.getChipModel(); };
    void        shutdown() override { reboot(); };
    void        reboot() override { ESP.restart(); };

    void setScreenBrightness(int8_t value) override { analogWrite(TFT_BL, (value * 255) / 100); };
    char getCharInput() override { return Serial.read(); };
    int  getKeyInput() override {
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
    };
    bool isCharging() override {
        if (charger_exists) { return bat.isChargerConnected(); }
    };
    void updateFrequencies() override {

    };
    RenderTarget* GetScreen() override { return setupTFTESPIRenderTarget(); }

  private:
    void initMCP() {
        mcp.writeRegister(MCP23017Register::GPIO_A, 0x00); // Reset port A
        mcp.writeRegister(MCP23017Register::GPIO_B, 0x00); // Reset port B
        ESP_LOGI("KEYPAD", "KEYPAD Initalized");
    }
    bool initSDcard(bool fast) {
        uint32_t freq  = fast ? FAST_SD_FREQ : SAFE_SD_FREQ;
        int      tries = 0;

        while (tries < 5) {
            if (freq >= getCpuFrequencyMhz() * 1000000) { freq /= 4; }
            ESP_LOGI("SD", "TRYING %lu Hz", freq);

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

    bool keypad_exists  = false;
    bool charger_exists = false;
    bool checkI2Cdevices(byte device) {
        Wire.beginTransmission(device);
        uint8_t error = Wire.endTransmission();
        return !error;
    }
};
#endif