#include "init.h"

IP5306      chrg;
TFT_eSPI    tft           = TFT_eSPI();
TFT_eSprite screen_buffer = TFT_eSprite(&tft);
Preferences preferences;
MCP23017    mcp = MCP23017(MCP23017_ADDR);
PNG         png;

SDImage mailimg[4] = {
    SDImage(0x662DB1, 18, 21, 0, true),
    SDImage(0x662DB1 + (18 * 21 * 2), 18, 21, 0, true),
    SDImage(0x662DB1 + (18 * 21 * 2 * 2), 18, 21, 0, true),
    SDImage(0x662DB1 + (18 * 21 * 2 * 3), 18, 21, 0, true)};

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
time_t    systemTime;
struct tm systemTimeInfo;

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

void setup() {
    setCpuFrequencyMhz(FAST_CPU_FREQ_MHZ);

    time(&systemTime);
    setenv("TZ", "UTC+1", 1);
    tzset();
    localtime_r(&systemTime, &systemTimeInfo);

    pinMode(TFT_BL, OUTPUT);
    analogWrite(TFT_BL, 0); // boot blinking prevention
    tft.init();
    tft.setAttribute(PSRAM_ENABLE, true);
    tft.fillScreen(0x0000);
    // INIT Serial
    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
    printT_S(String(ESP.getPsramSize()));
    // INIT charging IC
    chrg.begin(21, 22);

    // RESET KEYBOARD
    mcp.writeRegister(MCP23017Register::GPIO_A, 0x00); // Reset port A
    mcp.writeRegister(MCP23017Register::GPIO_B, 0x00); // Reset port B

    // set brightness
    analogWrite(TFT_BL, (255 * brightness) / 100);

    if (chrg.isChargerConnected() == 1) {
        offlineCharging();
        tft.setTextFont(1);
    }
    tft.fillScreen(0);
    tft.setCursor(0, 0);
progressBar(0, 100, 250);
    preferences.begin("settings", false);
    resPath = preferences.getString("resPath", resPath);
    Serial.println(resPath);

    SPI.begin(14, 2, 15, chipSelect);
    SPIFFS.begin();

    isSPIFFS = (!initSDCard(true) || !SD.exists(resPath)) && SPIFFS.exists(SPIFFSresPath);
    printT_S(isSPIFFS ? "Resource file or SD card is not available" : "Using resource card from SD card");
    if (!isSPIFFS && !SD.exists(resPath))
        recovery("Seems that you flashed your device wrong Try inserting SDcard with resource file");

    tft.println("LOADING RESOURCE FILE");
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
            SPIFFS.format();
            tft.println("Copying resource file...");

            File temp = SPIFFS.open(SPIFFSresPath, FILE_WRITE, true);
            temp.write(resources, SD.open(resPath).size() - RESOURCE_ADDRESS);
            temp.close();
        }
#ifndef LOG
    drawFromSd(50, 85, SDImage(0x665421, 140, 135)); // draw boot logo
#endif
    if (buttonsHelding(false) == '*')
        recovery("Manually triggered recovery."); // Chance to change resource file to custom one

    progressBar(20, 100, 250);
    tft.setCursor(12, 3);
    tft.setTextSize(3);
    tft.setTextColor(tft.color24to16(0x656565));
    printT_S("OkabePhone");
    tft.setCursor(15, 5);
    tft.setTextColor(0xFFFF);
    printT_S("OkabePhone");
#ifdef LOG
    tft.fillRect(0, 0, 10, 10, TFT_RED);
    tft.fillRect(0, 10, 10, 10, TFT_GREEN);
    tft.fillRect(0, 20, 10, 10, TFT_BLUE);
#endif
    tft.setTextSize(1);
    changeFont(0);
    printT_S("\nOkabePhone " + String(FIRMVER) + "\n\nPhone firmware written by Nergon\n\nResources located in sdcard\nfolder FIRMWARE\n");
    printT_S(String(ESP.getChipModel()) + String(" REV.") + String(ESP.getChipRevision()) + String(" ") + String(ESP.getCpuFreqMHz()) + String("MHz ") + String(ESP.getChipCores()) + String(" cores\n"));
#ifdef DEVMODE
    printT_S("\n       !!! DEVMODE ENABLED !!!\n\n");
#endif

    progressBar(30, 100, 250);
    uint32_t oldtime = millis();

    if (sendATCommand("AT").indexOf("OK") != -1) {
        printT_S("Setting up sim card please wait...");
        initSim();

        while (!_checkSim() && millis() - oldtime < 10000)
            ; // check if sim card is usable for 10 whole seconds...

        populateContacts();
        progressBar(90, 100, 250);
        printT_S("Done!");
    }

#ifdef SIMDEBUG
    AT_test();
#endif
    xTaskCreatePinnedToCore(
        TaskIdleHandler,
        "Core0Checker",
        10000,
        NULL,
        1,
        &TaskHCommand,
        0);

    progressBar(95, 100, 250);

    wallpaperIndex = preferences.getUInt("wallpaperIndex", 0);
    // Serial.println(String(wallpaperIndex) + "WI");
    //  contactCount   = preferences.getUInt("contactCount", 0);

    if (wallpaperIndex < 0 || wallpaperIndex > 42)
        currentWallpaperPath = preferences.getString("wallpaper", "/null");
    preferences.end();
    if (!SD.exists(currentWallpaperPath)) {
        printT_S(currentWallpaperPath + " - NOT FOUND");
    }
    progressBar(100, 100, 250);

    millSleep = millis();
    while (buttonsHelding() == '#')
        ;
    Serial.updateBaudRate(115200);
}

void suspendCore(bool suspend) {
    if (TaskHCommand)
        if (suspend) {
            vTaskSuspend(TaskHCommand);
            simIsBusy = false;
        } else
            vTaskResume(TaskHCommand);
}

void TaskIdleHandler(void *parameter) {
    while (true) {

        while (!simIsBusy && simIsUsable) {
            backgroundBusy = true;
            if (getSignalLevel() != _signal || getChargeLevel() != charge) {
                if (ongoingCall)
                    stateCall = GetState();
                _signal = getSignalLevel();
                charge  = getChargeLevel();
                Serial.println(charge);
                Serial.println("CHARGE:" + String(chrg.getBatteryLevel() / 25));
                simIsUsable = _checkSim();
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
void loop() {
    screens();
}
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
    tm sbtime = *localtime_r(&systemTime, &systemTimeInfo);
    if (sbtime.tm_min != systemTimeInfo.tm_min)
        drawStatusBar(true);

    checkVoiceCall();
    delay(50);
    if (millis() - millDelay > DBC_MS) {
        millDelay = millis();
        drawStatusBar();
    }
}
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
void initSim() {

    printT_S(sendATCommand("AT+CMEE=2"));
    progressBar(25, 100, 250);
    printT_S(sendATCommand("AT+CLIP=1"));
    progressBar(30, 100, 250);
    printT_S(sendATCommand("AT+CLCC=1"));
    progressBar(45, 100, 250);
    printT_S(sendATCommand("AT+CSCS=\"GSM\""));
    progressBar(50, 100, 250);
    printT_S(sendATCommand("AT+CMGF=1"));
    simIsUsable = _checkSim();
    progressBar(80, 100, 250);
}

bool initSDCard(bool fast) {
    SD.end();
    ulong sd_freq = MAX_SD_FREQ;
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
void loadResource(ulong address, String resourcefile, uint8_t **_resources, int w, int h) {
    File file;
    if (isSPIFFS)
        file = SPIFFS.open(SPIFFSresPath);
    else
        file = SD.open(resourcefile);
    if (!file) {
        Serial.println("Failed to open file!");
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
        Serial.println("Memory allocation failed!");
        file.close();
        return;
    }

    file.read(*_resources, size);
    file.close();
    Serial.printf("Loaded %d bytes from %s\n", size, resourcefile.c_str());
}