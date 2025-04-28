#include "init.h"
#include "images.h"

IP5306      chrg;
TFT_eSPI    tft           = TFT_eSPI();
TFT_eSprite screen_buffer = TFT_eSprite(&tft);
Preferences preferences;
MCP23017    mcp = MCP23017(MCP23017_ADDR);
PNG         png;



// pointer to the graphic resources in PSRAM
uint8_t *resources;

// Variable to check if status bar refresh required
bool sBarChanged = true;
// Variable that indicate if there incoming call
bool isCalling = false;
// Checking if background SIM card check in progress
bool backgroundBusy = false;
// Check if there any new messages (Status Bar indicator)
bool haveNewMessages = false;
// Check if SIM card can make calls
bool isAbleToCall = false;
// Check if someone answered our call
bool isAnswered = false;
// Check if SIM Serial in use
volatile bool simIsBusy = false;
// check if there call in progress
volatile bool ongoingCall = false;
// check if there SIM card available
volatile bool simIsUsable = false;

// spiffs?
bool isSPIFFS = false;

// current brightness in percentage
uint brightness = 100;

// Variable for non-blocking delay
int millDelay = 0;
// Current Screen variable, for recursion prevention
int currentScreen = SCREENS::MAINSCREEN;
// index of last Contact
int lastContactIndex = 0;
// count of all contacts
uint contactCount = 0;
// index of current wallpaper
uint32_t wallpaperIndex = 0;
// index of currentFont used for changeFont()
int currentFont = 0;

// clock
time_t systemTime;
tm     systemTimeInfo;

// is Screen Locked????
volatile bool isScreenLocked = false;

// time to sleep
ulong millSleep        = 0;
int   delayBeforeSleep = 60000;
int   delayBeforeLock  = 15000;
// Call state returned by SIM card
volatile int stateCall = 6;
// delay between SIM card checks
volatile int DBC_MS = 1000;

// Contacts storage
Contact contacts[MAX_CONTACTS];
// Handle for background SIM checks
TaskHandle_t TaskHCommand;

// Number of person who calling us
String currentNumber = "";

String currentRingtonePath     = "";
String currentMailRingtonePath = "";
String currentNotificationPath = "";
String currentWallpaperPath    = "/null";
String resPath                 = "/FIRMWARE/IMAGES.SG";
String SPIFFSresPath           = "/IMAGES_CUT.SG";
String lastSIMerror            = "";

void TaskIdleHandler(void *parameter);
void initSim();
bool initSDCard(bool fast);
void loadResource(ulong address, String resourcefile, uint8_t **_resources, int w, int h);

// Function to set up the time
// This function sets the system time to a specific date and time
// and saves it in the preferences storage
void SetUpTime() {

    struct tm tm_time = {};

    preferences.begin("TimePhone");

    systemTime         = preferences.getLong("TIME", mktime(&tm_time));
    struct timeval now = {.tv_sec = systemTime};
    settimeofday(&now, NULL);
    preferences.end();

    systemTimeInfo = *gmtime(&systemTime);
}

// Function to save the time
// This function saves the current time in the preferences storage
// and updates the system time
void SaveTime(time_t time) {
    preferences.begin("TimePhone");
    preferences.putLong("TIME", time);

    preferences.end();

    time_t         t    = time;
    struct timeval data = {t, 0};
    systemTimeInfo      = *gmtime(&time);
    // Serial.printf("\nSTI %d:%d, TIME: %d:%d\n", systemTimeInfo.tm_hour, systemTimeInfo.tm_min, tm_time.tm_hour, tm_time.tm_min);
    settimeofday(&data, NULL);
}

void restart_handler() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_RED);
    tft.setCursor(0, 0);
    tft.println("RESTARTING...");
    delay(100);
}

// Function to initialize the hardware components
void hardwareInit() {
    setCpuFrequencyMhz(FAST_CPU_FREQ_MHZ);
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
    Serial1.begin(SIM_BAUD_RATE, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
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
        SPIFFSres.close();
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

void initTasks() {
    xTaskCreatePinnedToCore(TaskIdleHandler, "Core0Checker", 10000, NULL, 1, &TaskHCommand, 0);
}

// initialize the system
void setup() {

    esp_register_shutdown_handler(restart_handler);

    SetUpTime();
    hardwareInit();

    if (chrg.isChargerConnected() == 1) {
        offlineCharging();
    }
    tft.fillScreen(0x0000);
    tft.setTextFont(1);
    tft.setCursor(0, 0);
    progressBar(0, 100, 250);

    if (buttonsHelding(false) == '*') {
        recovery("Manually triggered recovery."); // Chance to change resource file to custom one
    }
    storageInit();
    drawFromSd(50, 85, LOGO_IMAGE);
    initTasks();
    ESP_LOGI("DEVICE", "\nOkabePhone " FIRMVER "\n\nPhone firmware written by Nergon\n\nResources located in sdcard (%s)\n", resPath.c_str());
    ESP_LOGI("DEVICE", "%s REV.%u %u MHz %d cores", ESP.getChipModel(), ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getChipCores());
    progressBar(100, 100, 250);
    millSleep = millis();
    while (buttonsHelding(false) == '#')
        ;
    Serial.updateBaudRate(SERIAL_BAUD_RATE);

}

// Function to suspend/resume the freeRTOS task on core 0
void suspendCore(bool suspend) {
    if (TaskHCommand) {
        if (suspend) {
            vTaskSuspend(TaskHCommand);
            simIsBusy = false;
        } else
            vTaskResume(TaskHCommand);
    }
}

// Function to handle the idle task
void TaskIdleHandler(void *parameter) {

    uint32_t oldtime = millis();

    if (sendATCommand("AT").indexOf("OK") != -1) {
        printT_S("Setting up sim card please wait...");
        initSim();

        while (!_checkSim() && millis() - oldtime < 10000)
            ; // check if sim card is usable for 10 whole seconds...
        populateContacts();
        printT_S("Done!");
    } else {
        printT_S("SIM card not responding");
        simIsUsable = false;
    }

    while (true) {

        while (!simIsBusy && simIsUsable) {
            backgroundBusy = true;
            if (getSignalLevel() != _signal || getChargeLevel() != charge) {
                if (ongoingCall)
                    stateCall = GetState();
                _signal = getSignalLevel();
                charge  = getChargeLevel();
              
                simIsUsable = _checkSim();
                ESP_LOGI("SIGNAL", "Signal: %d, Charge: %d, SIM USABLE:%d\n", _signal, charge,simIsUsable);;
                if (contactCount == 0)
                    populateContacts();
                sBarChanged = true;
            }
            vTaskDelay(pdMS_TO_TICKS(DBC_MS));
            backgroundBusy = false;
        }
        if (!simIsUsable) {
            DBC_MS      = 10000;
            simIsUsable = _checkSim();
            charge      = getChargeLevel();
            if (_signal != -1) {
                _signal     = -1;
                sBarChanged = true;
            }

        } else {
            DBC_MS = 3000;
        }
        backgroundBusy = false;
        vTaskDelay(pdMS_TO_TICKS(DBC_MS));
    }
}

// Function to handle the main loop
void loop() {
    screens();
}

// Function to handle the idle state
void idle() {

    if (millis() > millSleep + (delayBeforeSleep / 2) && millis() < millSleep + delayBeforeSleep) {
        setBrightness(brightness * 0.1);
    } else if (millis() > millSleep + delayBeforeSleep) {
        setBrightness(0);
    } else {
        setBrightness(brightness);
    }
    if (millis() > millSleep + delayBeforeSleep + delayBeforeLock && !isScreenLocked) {
        LockScreen();
    }

    drawStatusBar(false);

    checkVoiceCall();
    delay(50);
    if (millis() - millDelay > DBC_MS) {
        millDelay = millis();
        drawStatusBar();
    }
}

// Function to handle the screens
// This function is responsible for displaying the current screen
void screens() {
    switch (currentScreen) {
    case SCREENS::MAINSCREEN:
        MainScreen();
        break;
    case SCREENS::MAINMENU:
        MainMenu();
        break;
    case SCREENS::MESSAGES:
        messages();
        break;
    case SCREENS::CONTACTS:
        contactss();
        break;
    case SCREENS::SETTINGS:
        settings();
        break;
    case SCREENS::E:
        e();
        break;
    }
}

// Function to initialize the SIM card
// This function sends AT commands to the SIM card to set it up
void initSim() {
    printT_S(sendATCommand("AT+CMEE=2"));
    printT_S(sendATCommand("AT+CLIP=1"));
    printT_S(sendATCommand("AT+CLCC=1"));
    printT_S(sendATCommand("AT+CSCS=\"GSM\""));
    printT_S(sendATCommand("AT+CMGF=1"));
    simIsUsable = _checkSim();
}

// Function to initialize the SD card
// This function initializes the SD card and sets the frequency
bool initSDCard(bool fast) {
    SD.end();
    uint32_t sd_freq = MAX_SD_FREQ;
    if (!SD.begin(chipSelect, SPI, SAFE_SD_FREQ))
        return false;
    else
        SD.end();
    if (fast) {
        while (sd_freq >= MIN_SD_FREQ) {
            if (SD.begin(chipSelect, SPI, sd_freq)) {
                return true;
            }
            sd_freq -= 1000000;
        }
    }

    return SD.begin(chipSelect, SPI, SAFE_SD_FREQ);
}

// Function to load a resource file from the SD card to the PSRAM
void loadResource(ulong address, String resourcefile, uint8_t **_resources, int w, int h) {
#ifdef PSRAM_ENABLE
    File file;
    if (isSPIFFS)
        file = SPIFFS.open(SPIFFSresPath);
    else
        file = SD.open(resourcefile);
    if (!file) {
        ESP_LOGE("FS","Failed to open file!");
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
        ESP_LOGE("MALLOC","Memory allocation failed!");
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