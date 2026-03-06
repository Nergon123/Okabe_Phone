#include "GlobalVariables.h"
TFT_STUB    tft;
Preferences preferences;

int8_t _signal = 0;
int8_t charge  = 0;

bool enableRAMMonitor = false;

// pointer to the graphic resources in PSRAM
uint8_t* resources;

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

// is Screen Locked????
volatile bool isScreenLocked = false;

// time to sleep
ulong millSleep        = 0;
int   delayBeforeSleep = 60000;
int   delayBeforeLock  = 15000;
// Call state returned by SIM card
volatile int stateCall = 6;

bool mcpexists    = false;
bool ip5306exists = false;

// Contacts storage
std::vector<Contact> contacts;

VirtualFileSystem VFS;

// forbid to change fastMode
bool          lockFastMode         = false;
RenderTarget* currentRenderTarget  = nullptr;
RenderTarget* bufferedRenderTarget = nullptr;

NString currentNumber           = "";
NString currentRingtonePath     = "";
NString currentMailRingtonePath = "";
NString currentNotificationPath = "";
NString currentWallpaperPath    = "";
NString resPath                 = "/spiffs/MAIN.nph";
NString SPIFFSresPath           = "/MAIN.nph";
NString lastSIMerror            = "";

std::vector<KeypadLayout> keypadLayouts = {
    KeypadLayout(SMALL_LATIN,
                 {" 0+\n", ".,?!'\"1-()@/:_", "abcà2", "defèé3", "ghiì4", "jkl5", "mnoò6", "pqrs7",
                  "tuvù8", "wxyz9"},
                 "Lowercase latin", "abc", nullptr, true),
    KeypadLayout(CAPS_LATIN,
                 {" 0+\n", ".,?!'\"1-()@/:_", "ABCÀ2", "DEFÈÉ3", "GHIÌ4", "JKL5", "MNOÒ6", "PQRS7",
                  "TUVÙ8", "WXYZ9"},
                 "Uppercase latin", "ABC", nullptr, true),

    KeypadLayout(NUMBERS, {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"}, "Numbers", "123",
                 nullptr, true)
};

iHW* hw;

PackageManager pm;
PackageStorage PaStor;

#ifdef TASK
std::vector<TASK> tasks;
#endif
#ifdef PC
int SDLScale = 1;
#endif
