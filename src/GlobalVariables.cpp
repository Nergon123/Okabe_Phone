#include "GlobalVariables.h"
IP5306      chrg;
TFT_STUB   tft           ;
Preferences preferences;

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
uint8_t brightness = 100;

int currentScreen = SCREENS::MAINSCREEN;
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


bool mcpexists = false;
bool ip5306exists = false;

// Contacts storage
std::vector<Contact> contacts;
// Handle for background SIM checks


VirtualFileSystem VFS;
// forbid to change fastMode
bool lockFastMode = false;
RenderTarget* currentRenderTarget = nullptr;
RenderTarget* bufferedRenderTarget = nullptr;

NString currentNumber           = "";
NString currentRingtonePath     = "";
NString currentMailRingtonePath = "";
NString currentNotificationPath = "";
NString currentWallpaperPath    = "";
NString resPath                 = "/spiffs/MAIN.nph";
NString SPIFFSresPath           = "/MAIN.nph";
NString lastSIMerror            = "";

#ifdef PC
int SDLScale = 1;
#endif