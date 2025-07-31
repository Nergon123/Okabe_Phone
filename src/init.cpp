#include "init.h"
#include "System/ResourceSystem.h"
// Function to set up the time
// This function sets the system time to a specific date and time
// and saves it in the preferences storage

// Function to initialize the SD card
// This function initializes the SD card and sets the frequency
// @param fast If true, set the frequency to the maximum value
// @return true if the SD card is initialized successfully, false otherwise
bool initSDCard(bool fast) {
    SD.end();
    uint32_t freq;
    int tries = 0;
    if (fast) {
        freq = FAST_SD_FREQ;
        if (freq >= getCpuFrequencyMhz() * 1000000) {
            freq /= 4;
        }
        if (SD.begin(chipSelect, SPI, freq)) {
            return true;
        } else {
            SD.end();
            freq = SAFE_SD_FREQ;
            while (freq >= getCpuFrequencyMhz() * 1000000) {
                freq /= 4;
                tries++;
                if(tries>5)break;
            }
            return SD.begin(chipSelect, SPI, freq);
        }
    } else {
        freq = SAFE_SD_FREQ;
        while (freq >= getCpuFrequencyMhz() * 1000000) {
            freq /= 4;
            if(tries>5)break;
        }
        SD.end();
        return SD.begin(chipSelect, SPI, freq);
    }
    return false;
}

// Function to initialize the hardware components
void hardwareInit() {
    setCpuFrequencyMhz(SLOW_CPU_FREQ_MHZ);
    // INIT charging IC as well as I2C
    chrg.begin(21, 22);
    pinMode(TFT_BL, OUTPUT);
    analogWrite(TFT_BL, 0); // boot blinking prevention
    tft.init();
#ifdef PSRAM_ENABLE
    tft.setAttribute(PSRAM_ENABLE, true);
#else
    ESP_LOGW("PSRAM", "PSRAM DISABLED");
#endif
    tft.fillScreen(0x0000);

    // INIT Serial
    Serial.begin(SERIAL_BAUD_RATE);
    ESP_LOGI("SERIAL", "Serial started at %d baud", SERIAL_BAUD_RATE);
    SimSerial.begin(SIM_BAUD_RATE, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);

    ESP_LOGI("SIM_CARD_SERIAL", "Serial1 (SIM CARD COMMUNICATION) started");
#ifdef PSRAM_ENABLE
    ESP_LOGI("PSRAM", "PSRAM SIZE: %d bytes", ESP.getPsramSize());
#else
    ESP_LOGW("PSRAM", "PSRAM DISABLED");
#endif
    // RESET KEYBOARD
    mcp.writeRegister(MCP23017Register::GPIO_A, 0x00); // Reset port A
    mcp.writeRegister(MCP23017Register::GPIO_B, 0x00); // Reset port B
    ESP_LOGI("KEYPAD", "KEYPAD RESET");
    SPI.begin(14, 2, 15, chipSelect);
    ESP_LOGI("SD", "SPI started");
    SPIFFS.begin();
    ESP_LOGI("SPIFFS", "SPIFFS started");
    // set brightness
    analogWrite(TFT_BL, (255 * brightness) / 100);
}

// Function to initialize the SD card and check if it is available
void storageInit() {
    preferences.begin("settings", false);
    resPath  = preferences.getString("resPath", resPath);
    isSPIFFS = (!initSDCard(true) || !SD.exists(resPath)) && SPIFFS.exists(SPIFFSresPath);
    if (isSPIFFS) {
        ESP_LOGI("RESOURCES", "Resource file or SD card is not available");
    } else {
        ESP_LOGI("RESOURCES", "Using resource file from SD card");
    }
    if (!isSPIFFS && !SD.exists(resPath))
        recovery(SplitString("Seems that you flashed your device wrongly.Refer to the instructions for more information."));

    ESP_LOGI("RESOURCES", "LOADING RESOURCE FILE");
    progressBar(10, 100, 250);



    if (!isSPIFFS) {
        loadResource(RESOURCE_ADDRESS, resPath, &resources, 0, 0);
    } else {
        File SPIFFSres = SPIFFS.open(SPIFFSresPath);
        loadResource(0, SPIFFSresPath, &resources, 0, 0);
        res.Init(SPIFFSres);
    }

    if (!isSPIFFS)
        if (SPIFFS.open(SPIFFSresPath, FILE_READ).size() != SD.open(resPath).size() - RESOURCE_ADDRESS) {
            tft.println("Outdated file, formatting SPIFFS...");
            SPIFFS.format();
            tft.println("Copying resource file...");

            File temp = SPIFFS.open(SPIFFSresPath, FILE_WRITE, true);
            temp.write(resources, SD.open(resPath).size() - RESOURCE_ADDRESS);
            temp.close();
        }

    wallpaperIndex = preferences.getUInt("wallpaperIndex", 0);
    if (wallpaperIndex < 0 || wallpaperIndex > 42)
        currentWallpaperPath = preferences.getString("wallpaper", "/null");
    preferences.end();
    if (!SD.exists(currentWallpaperPath)) {
        ESP_LOGI("SDCARD", "%s - NOT FOUND", currentWallpaperPath.c_str());
    }
    progressBar(70, 100, 250);
}




// Function to load a resource file from the SD card to the PSRAM
// @param address: address in the file
// @param resourcefile: path to the resource file
// @param _resources: pointer to the resources buffer
// @param w: width of the image
// @param h: height of the image
void loadResource(ulong address, String resourcefile, uint8_t **_resources, int w, int h) {

    ////////////////////////////
    return;
    //we dont need this function for now I guess...
    ////////////////////////////

    #ifdef PSRAM_ENABLE
    if (*_resources)
        free(*_resources);
    File file;
    if (isSPIFFS)
        file = SPIFFS.open(SPIFFSresPath);
    else
        file = SD.open(resourcefile);
    if (!file) {
        ESP_LOGE("FS", "Failed to open file!");
        return;
    }

    file.seek(address);
    size_t size;
    if (w == 0 && h == 0)
        size = file.size() - address;
    else
        size = w * h * 2;

    *_resources = (uint8_t *)ps_malloc(size);

    if (!(*_resources)) {
        ESP_LOGE("MALLOC", "Memory allocation failed!");
        sysError("MEMORY_ALLOCATION_FAILED" + address);
        file.close();
        return;
    }

    file.read(*_resources, size);
    file.close();
    ESP_LOGI("PSRAM", "Loaded %d bytes from %s\n", size, resourcefile.c_str());
#else
    ESP_LOGW("PSRAM", "PSRAM not enabled ! ! !");
#endif
}