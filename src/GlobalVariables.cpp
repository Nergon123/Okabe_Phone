#include "GlobalVariables.h"
IP5306      chrg;
TFT_eSPI    tft           = TFT_eSPI();
TFT_eSprite screen_buffer = TFT_eSprite(&tft);
MCP23017    mcp           = MCP23017(MCP23017_ADDR);
Preferences preferences;
PNG         png;

int8_t _signal = 0;
int8_t charge  = 0;

// pointer to the graphic resources in PSRAM
uint8_t *resources;

// Variable to check if status bar refresh required
bool sBarChanged = true;

// Checking if background SIM card check in progress
bool backgroundBusy = false;
// Check if there any new messages (Status Bar indicator)
bool haveNewMessages = false;

// is spiffs used?
bool isSPIFFS = false;
// current brightness in percentage
uint brightness = 100;

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
Contact contacts[MAX_CONTACTS]; // TODO : convert array to vector
// Handle for background SIM checks
TaskHandle_t TaskHCommand;

// forbid to change fastMode
bool lockFastMode = false;
// Number of person who calling us
String currentNumber = "";

String currentRingtonePath     = "";
String currentMailRingtonePath = "";
String currentNotificationPath = "";
String currentWallpaperPath    = "/null";
String resPath                 = "/FIRMWARE/IMAGES.SG";
String SPIFFSresPath           = "/MAIN.nph";
String lastSIMerror            = "";