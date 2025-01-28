#include "init.h"

IP5306      chrg;
TFT_eSPI    tft = TFT_eSPI();
Preferences preferences;
MCP23017    mcp = MCP23017(MCP23017_ADDR);
PNG         png;

SDImage mailimg[4] = {
    SDImage(0x662DB1, 18, 21, 0, true),
    SDImage(0x662DB1 + (18 * 21 * 2), 18, 21, 0, true),
    SDImage(0x662DB1 + (18 * 21 * 2 * 2), 18, 21, 0, true),
    SDImage(0x662DB1 + (18 * 21 * 2 * 3), 18, 21, 0, true)};

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

// Call state returned by SIM card
volatile int stateCall = 6;
// delay between SIM card checks
volatile int DBC_MS = 1000;

// Contacts storage
Contact contacts[MAX_CONTACTS];
// Default contact (to be erased)
Contact examplecontact = {0, "+1234567890", "NERGON", "petro.chazov@gmail.com"};

// Handle for background SIM checks
TaskHandle_t TaskHCommand;

// Number of person who calling us
String currentNumber = "";

String currentRingtonePath     = "";
String currentMailRingtonePath = "";
String currentNotificationPath = "";
String currentWallpaperPath    = "/null";
void   TaskIdleHandler(void *parameter);
void   initSim();
String lastSIMerror = "";
void   setup() {
    mcp.writeRegister(MCP23017Register::GPIO_A, 0x00); // Reset port A
    mcp.writeRegister(MCP23017Register::GPIO_B, 0x00); // Reset port B

    tft.init();
    tft.fillScreen(0x0000);
    // INIT PINS
    analogWrite(TFT_BL, 1024);

    pinMode(38, INPUT_PULLUP);
    pinMode(0, INPUT_PULLUP);
    pinMode(39, INPUT_PULLUP);
    pinMode(37, INPUT_PULLUP);

    chrg.begin(21, 22);

    /*if (chrg.isChargerConnected() == 1)
    {
      offlineCharging();
      tft.setTextFont(1);
    } */
    SPI.begin(14, 2, 15, chipSelect);
#ifndef LOG
    if (!SD.begin(chipSelect, SPI, 80000000))
        sysError("SD_CARD_INIT_FAIL");
    if (!SD.exists("/FIRMWARE/IMAGES.SG"))
        recovery("No /FIRMARE/IMAGES.SG found");
    drawFromSd(50, 85, SDImage(SDImage(0x665421, 140, 135)));
#endif
    progressBar(0, 100,250);
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

#ifdef DEVMODE
    printT_S("\n       !!! DEVMODE ENABLED !!!\n\n       THIS MEANS THAT THIS \n       BUILD NOT FOR PRODUCTION\n");
#endif

    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
    progressBar(10, 100,250);
    uint32_t oldtime = millis();
    if (sendATCommand("AT").indexOf("OK") != -1) {
        printT_S("Setting up sim card please wait...");
        initSim();
        while (!_checkSim() && millis() - oldtime < 10000)
            ;
        populateContacts();
            progressBar(90, 100,250);
        printT_S("Done!");
    }

#ifdef SIMDEBUG
    if (digitalRead(39) == LOW) {

        tft.fillScreen(0);
        tft.setCursor(0, 0);
        tft.println("Waiting for connection...");

        while (true) {
            if (Serial.available()) {
                tft.fillScreen(0);
                tft.setCursor(0, 0);
                tft.setTextSize(2);

                tft.setTextColor(0xF800);
                tft.println("\nAT COMMANDS CONSOLE\n (TO EXIT TYPE :q)\n");
                tft.setTextColor(0xFFFF);
                tft.setTextSize(1);
                String req;

                tft.println("REQUEST:");
                req = Serial.readString();
                tft.print(req + "\n");
                if (req.indexOf(":q") != -1) {
                    break;
                }
                String ans = sendATCommand(req);

                tft.println("\nANSWER: " + ans);
                Serial.println(ans);
            }
        }
    }
#endif
    xTaskCreatePinnedToCore(
        TaskIdleHandler,
        "Core0Checker",
        10000,
        NULL,
        1,
        &TaskHCommand,
        0);

    printT_S("Initializing SD card...");
#ifdef LOG

    if (!SD.begin(chipSelect, SPI, 80000000)) {

        tft.setTextColor(0xf800);
        printT_S("\nSD Initialization failed!");

        delay(1000);
        // recovery("Something went wrong with your sd card\n(Possibly its just not there)\n");
        // I can't work without sd card >_<
        sysError("SD_CARD_INIT_FAIL");
    } else {
        printT_S("SD Initialization done.");
    }
    if (!SD.exists("/FIRMWARE/IMAGES.SG"))
        recovery("No /FIRMARE/IMAGES.SG found");
#endif
        progressBar(95, 100,250);
    preferences.begin("settings", false);
    wallpaperIndex = preferences.getUInt("wallpaperIndex", 0);
    // contactCount   = preferences.getUInt("contactCount", 0);
    if (wallpaperIndex < 0 || wallpaperIndex > 42)
        currentWallpaperPath = preferences.getString("wallpaper", "/null");
    if (!SD.exists(currentWallpaperPath)) {
        wallpaperIndex = 0;
        printT_S(currentWallpaperPath + " - NOT FOUND");
    }
    progressBar(100, 100,250);
    // if (!SD.exists("/DATA/MESSAGES.JSON")) {
    //     if (!SD.exists("/DATA")) {
    //         SD.mkdir("/DATA");
    //     }
    //     File file = SD.open("/DATA/MESSAGES.JSON", FILE_WRITE);
    //     file.print("{}");
    //     file.close();
    // }


    while (digitalRead(37) == LOW)
        ;
    drawStatusBar();
}
void suspendCore(bool suspend) {
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
    progressBar(25, 100,250);
    printT_S(sendATCommand("AT+CLIP=1"));
    progressBar(30, 100,250);
    printT_S(sendATCommand("AT+CLCC=1"));
    progressBar(45, 100,250);
    printT_S(sendATCommand("AT+CSCS=\"GSM\""));
    progressBar(50, 100,250);
    printT_S(sendATCommand("AT+CMGF=1"));
    simIsUsable = _checkSim();
    progressBar(80, 100,250);
}