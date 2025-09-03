#pragma once
#include "Defines.h"
#include "ExtDep/IP5306.h"
#include "esp_heap_caps.h"

#include <Arduino.h>
#include <MCP23017.h>
#include <PNGdec.h>
#include <Preferences.h>
#include <SD.h>
#include <TFT_eSPI.h>

// hmm....
extern TFT_eSprite screen_buffer;
#include "System/ResourceSystem.h"

struct Contact {
    int    index;
    String phone;
    String name;
    String email;
    Contact() : index(-1), phone(""), name(""), email("") {}
    Contact(String _name, String _phone, String _email = "", int _index = -1) : index(_index), phone(_phone), name(_name), email(_email) {}
};

// Options for list Menu
struct mOption {

    String   label;
    uint16_t icon;
    uint8_t  icon_index;
    uint8_t  fileId;
};

// SMS status
enum status {

    NEW     = 'N',
    REPLIED = 'R',
    READED  = 'D'
};

struct Message {
    int     index;
    int     status;
    bool    isOutgoing;
    Contact contact;
    String  subject;
    String  content;
    String  date;
    String  longdate;

    operator mOption() const {
        return mOption{date + " " + contact.name, .icon = R_LIST_MAIL_ICONS, .icon_index = (status == status::NEW ? (uint8_t)0 : (uint8_t)1), .fileId = 0};
    }

    Message(Contact _contact, String _subject, String _content, String _date, String _longdate, bool _isOutgoing = false, unsigned char _status = status::NEW, int _index = -1)
        : index(_index), status(_status), isOutgoing(_isOutgoing), contact(_contact), subject(_subject), content(_content), date(_date), longdate(_longdate) {}

    Message()
        : index(-1), status(status::NEW), isOutgoing(false), contact(Contact()), subject(""), content(""), date("00/00"), longdate("00/00/00 00:00") {}
};
struct STR_DIR {
    String text;
    int    direction;
};

enum SCREENS {
    MAINSCREEN,
    MAINMENU,
    MESSAGES,
    CONTACTS,
    SETTINGS,
    E,
};

extern MCP23017 mcp;
extern IP5306   chrg;
extern TFT_eSPI tft;

extern Preferences preferences;
extern PNG         png;

extern uint          contactCount;
extern int           currentScreen;
extern int           currentFont;
extern int           lastContactIndex;
extern int           delayBeforeSleep;
extern int           delayBeforeLock;
extern int8_t        _signal;
extern int8_t        charge;
extern uint          brightness;
extern uint32_t      wallpaperIndex;
extern ulong         millSleep;
extern volatile int  DBC_MS;
extern volatile int  stateCall;
extern bool          sBarChanged;
extern bool          isAbleToCall;
extern bool          isCalling;
extern bool          isAnswered;
extern bool          backgroundBusy;
extern bool          lockFastMode;
extern volatile bool isScreenLocked;
extern volatile bool simIsBusy;
extern volatile bool ongoingCall;
extern volatile bool simIsUsable;
extern Contact       contacts[MAX_CONTACTS];
extern Contact       examplecontact;

extern String lastSIMerror;
extern String currentNumber;
extern String currentRingtonePath;
extern String currentNotificationPath;
extern String currentMailRingtonePath;
extern String currentWallpaperPath;
extern String resPath;
extern String SPIFFSresPath;

extern uint8_t *resources;

extern bool isSPIFFS;

extern TaskHandle_t TaskHCommand;

// Function to get the size of an array ... bruh
template <typename T, size_t N>
size_t ArraySize(T (&)[N]) {
    return N;
}

extern time_t systemTime;
extern tm     systemTimeInfo;
