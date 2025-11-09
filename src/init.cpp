#include "init.h"
#include "System/ResourceSystem.h"
#ifdef PC
#include "Platform/Graphics/SDL2RenderTarget.h"
#else
#include "Platform/Graphics/TFTESPIRenderTarget.h"
#endif

bool checkI2Cdevices(byte device) {
#ifndef PC
    Wire.beginTransmission(device);
    uint8_t error = Wire.endTransmission();
    return !error;
#else
    (void)device;
#endif
    return false;
}

// Function to initialize the SD card
// This function initializes the SD card and sets the frequency
// @param fast If true, set the frequency to the maximum value
// @return true if the SD card is initialized successfully, false otherwise
bool initSDCard(bool fast) {
#ifndef PC
    uint32_t freq  = fast ? FAST_SD_FREQ : SAFE_SD_FREQ;
    int      tries = 0;

    while (tries < 5) {
        if (freq >= getCpuFrequencyMhz() * 1000000) { freq /= 4; }
        ESP_LOGI("SD", "TRYING %lu Hz", freq);

        if (SD.begin(chipSelect, SPI, freq)) {
            IFileSystem* sd = new Esp32FileSystem(&SD);
            VFS.mount("/sd", sd);
            return true;
        }
        freq /= 2;
        tries++;
    }
    return false; // failed to init
#else
    (void)fast;
    IFileSystem* sdcard = new StdFileSystem("sd/",FS_EXTERNAL);
    if (sdcard->begin()) {
        VFS.mount("/sd", sdcard);
        return true;
    }
#endif
    return false;
}

// Function to initialize the hardware components
void hardwareInit() {

#ifndef PC
    setCpuFrequencyMhz(FAST_CPU_FREQ_MHZ);
    // INIT charging IC as well as I2C
    mcpexists    = checkI2Cdevices(MCP23017_ADDR);
    ip5306exists = checkI2Cdevices(IP5306_ADDR);

    pinMode(TFT_BL, OUTPUT);
    analogWrite(TFT_BL, 0); // boot blinking prevention
#endif
#ifdef PC
    currentRenderTarget = setupSDL2RenderTarget(240, 320, "Okabe Phone Emulator");
#else
    currentRenderTarget = setupTFTESPIRenderTarget();
#endif
    tft.setRenderTarget(currentRenderTarget);
    tft.init();
#ifndef PC
    tft.fillScreen(0x0000);
    // INIT Serial
    Serial.begin(SERIAL_BAUD_RATE);
    ESP_LOGI("SERIAL", "Serial initalized at %d baud", SERIAL_BAUD_RATE);
    SimSerial.begin(SIM_BAUD_RATE, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
    ESP_LOGI("SIM_CARD_SERIAL", "Sim Card Serial initialized at %d", SIM_BAUD_RATE);

    // RESET KEYBOARD

    SPI.begin(14, 2, 15, chipSelect);
    ESP_LOGI("SD", "SPI started");

    // set brightness
    analogWrite(TFT_BL, (255 * brightness) / 100);
#endif
}

// Function to initialize the storage
void storageInit() {

#ifndef PC
    SPIFFS.begin();
    IFileSystem* spiffs = new Esp32FileSystem(&SPIFFS, FS_INTERNAL);
    IFileSystem* sdcard = new Esp32FileSystem(&SD, FS_EXTERNAL);
#else
    IFileSystem* spiffs = new StdFileSystem("spiffs/", FS_INTERNAL);
    IFileSystem* sdcard = new StdFileSystem("sd/", FS_EXTERNAL);
#endif

    sdcard->begin();
    VFS.mount("/sd", sdcard);
    spiffs->begin();
    VFS.mount("/spiffs", spiffs);
    ESP_LOGI("SPIFFS", "SPIFFS started");

    preferences.begin("settings", false);
    resPath = preferences.getString("resPath", resPath.c_str());

    if (!VFS.exists(resPath)) {
        recovery(SplitString("Seems that you flashed your device wrongly.Refer to the "
                             "instructions for more information."));
    }

    ESP_LOGI("RESOURCES", "LOADING RESOURCE FILE");
    progressBar(10, 100, 250);
    bootText("Loading resource file...");
    if (!res.Files[RES_MAIN]) {
        NFile* Resource = VFS.open(resPath);
        res.Init(Resource);
    }
    if (!res.Files[RES_MAIN]) { recovery("There was an error when loading resource file."); }

    currentWallpaperPath = preferences.getString("wallpaper", "");

    if (!VFS.exists(currentWallpaperPath)) {
        wallpaperIndex = preferences.getInt("wallpaperIndex", 0);
    }

    preferences.end();
    progressBar(70, 100, 250);
}
