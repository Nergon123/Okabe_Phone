#include "init.h"
#include "System/ResourceSystem.h"

bool checkI2Cdevices(byte device) {
    Wire.beginTransmission(device);
    uint8_t error = Wire.endTransmission();
    return !error;
}

// Function to initialize the SD card
// This function initializes the SD card and sets the frequency
// @param fast If true, set the frequency to the maximum value
// @return true if the SD card is initialized successfully, false otherwise
bool initSDCard(bool fast) {
    SD.end();
    uint32_t freq;
    int      tries = 0;
    if (fast) {
        freq = FAST_SD_FREQ;
        if (freq >= getCpuFrequencyMhz() * 1000000) { freq /= 4; }
        if (SD.begin(chipSelect, SPI, freq)) { return true; }
        else {
            SD.end();
            freq = SAFE_SD_FREQ;
            while (freq >= getCpuFrequencyMhz() * 1000000) {
                freq /= 4;
                tries++;
                if (tries > 5) { break; }
            }
            return SD.begin(chipSelect, SPI, freq);
        }
    }
    else {
        freq = SAFE_SD_FREQ;
        while (freq >= getCpuFrequencyMhz() * 1000000) {
            freq /= 4;
            if (tries > 5) { break; }
        }
        SD.end();
        return SD.begin(chipSelect, SPI, freq);
    }
    return false;
}

// Function to initialize the hardware components
void hardwareInit() {
    setCpuFrequencyMhz(FAST_CPU_FREQ_MHZ);
    // INIT charging IC as well as I2C
    chrg.begin(21, 22);
    mcpexists    = checkI2Cdevices(MCP23017_ADDR);
    ip5306exists = checkI2Cdevices(IP5306_ADDR);

    pinMode(TFT_BL, OUTPUT);
    analogWrite(TFT_BL, 0); // boot blinking prevention
    tft.init();
    tft.fillScreen(0x0000);

    if (psramFound()) {
        tft.setAttribute(PSRAM_ENABLE, true);
        ESP_LOGI("PSRAM", "PSRAM SIZE: %d bytes", ESP.getPsramSize());
    }
    else { ESP_LOGW("PSRAM", "PSRAM DISABLED"); }

    // INIT Serial
    Serial.begin(SERIAL_BAUD_RATE);
    ESP_LOGI("SERIAL", "Serial initalized at %d baud", SERIAL_BAUD_RATE);
    SimSerial.begin(SIM_BAUD_RATE, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
    ESP_LOGI("SIM_CARD_SERIAL", "Sim Card Serial initialized at %d", SIM_BAUD_RATE);

    // RESET KEYBOARD
    mcp.writeRegister(MCP23017Register::GPIO_A, 0x00); // Reset port A
    mcp.writeRegister(MCP23017Register::GPIO_B, 0x00); // Reset port B
    ESP_LOGI("KEYPAD", "KEYPAD Initalized");

    SPI.begin(14, 2, 15, chipSelect);
    ESP_LOGI("SD", "SPI started");

    // set brightness
    analogWrite(TFT_BL, (255 * brightness) / 100);
}

// Function to initialize the storage
void storageInit() {

    if (isSPIFFS) { bootText(isSPIFFS ? "SPIFFS enabled" : "SPIFFS disabled"); }

    SPIFFS.begin();
    ESP_LOGI("SPIFFS", "SPIFFS started");

    preferences.begin("settings", false);
    resPath = preferences.getString("resPath", resPath);

    isSPIFFS = (!initSDCard(true) || !SD.exists(resPath)) && SPIFFS.exists(SPIFFSresPath);

    if (isSPIFFS) {
        ESP_LOGI("RESOURCES", "Resource file or SD card is not available. Trying SPIFFS.");
    }
    else { ESP_LOGI("RESOURCES", "Using resource file from SD card"); }

    if (!isSPIFFS && !SD.exists(resPath)) {
        recovery(SplitString("Seems that you flashed your device wrongly.Refer to the "
                             "instructions for more information."));
    }

    ESP_LOGI("RESOURCES", "LOADING RESOURCE FILE");
    progressBar(10, 100, 250);
    bootText("Loading resource file...");

    File Resource;
    if (!isSPIFFS) { Resource = SD.open(resPath); }
    else { Resource = SPIFFS.open(SPIFFSresPath); }

    if (Resource) { res.Init(Resource); }
    else { recovery("There was an error when loading resource file."); }

    currentWallpaperPath = preferences.getString("wallpaper", "");

    if (!SD.exists(currentWallpaperPath)) {
        wallpaperIndex = preferences.getUInt("wallpaperIndex", 0);
    }

    preferences.end();
    progressBar(70, 100, 250);
}
