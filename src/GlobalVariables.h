#pragma once
#include "Defines.h"

#ifdef PC
#include <thread>
#else
#include <Arduino.h>
#include <MCP23017.h>
#include <TFT_eSPI.h>
#endif
#include "Platform/FileSystem/VFS.h"
#include "Platform/Graphics/ImageProcessor.h"
#include "Platform/Hardware/Hardware.h"
#include "Platform/NString.h"
#include "Platform/Preferences.h"

#include <vector>
// hmm....
#include "System/ResourceSystem.h"
#include "System/Pkg/PackageManager.h"
#include "System/Pkg/PackageStorage.h"

struct Contact {
    int     index;
    NString phone;
    NString name;
    NString email;
    Contact(NString _name = "", NString _phone = "", NString _email = "", int _index = -1)
        : index(_index), phone(_phone), name(_name), email(_email) {}
};

// what listMenu returns
struct LM_RET_VALUE {
    int index;
    int button;
        operator int() { return index; }
    LM_RET_VALUE(int index, int button = -1) : index(index), button(button) {};
};

// Options for list Menu
struct mOption {
    NString label;
    Image   image;
    uint8_t icon_index;
    void (*_function)();
    mOption(NString label, Image image = Image(), uint8_t icon_index = 0,
            void (*_function)() = nullptr)
        : label(label), image(image), icon_index(icon_index), _function(_function) {};
};

// SMS status
enum status { NEW = 'N', REPLIED = 'R', READED = 'D' };

struct Message {
    int     index;
    int     status;
    bool    isOutgoing;
    Contact contact;
    NString subject;
    NString content;
    NString date;
    NString longdate;
    int     part;
    int     total;
            operator mOption() const {
        return mOption(date + " " + contact.name, Image(R_LIST_MAIL_ICONS),
                       status == status::NEW ? (uint8_t)0 : (uint8_t)1);
    }

    Message(Contact _contact = Contact(), NString _subject = "", NString _content = "",
            NString _date = "00/00", NString _longdate = "00/00/00 00:00",
            bool _isOutgoing = false, unsigned char _status = status::NEW, int _index = -1)
        : index(_index), status(_status), isOutgoing(_isOutgoing), contact(_contact),
          subject(_subject), content(_content), date(_date), longdate(_longdate), part(0),
          total(1) {}
};
struct STR_DIR {
    NString text;
    int     direction;
};

enum SCREENS {
    MAINSCREEN,
    MAINMENU,
    MESSAGES,
    CONTACTS,
    SETTINGS,
    E,
};
enum LM_TYPE {
    LM_MESSAGES = 0,
    LM_SETTINGS = 1,
    LM_CONTACTS = 2,
};

enum LIST_ICONS {
    LM_ICO_FILE             = 0,
    LM_ICO_FOLDER           = 1,
    LM_ICO_SDCARD           = 2,
    LM_ICO_INTERNAL_STORAGE = 3,
    LM_ICO_IMAGE            = 4,
    LM_ICO_AUDIO            = 5,
    LM_ICO_THEME            = 6,
    LM_ICO_TEXT             = 7,
    LM_ICO_CHECK_UNCHECKED  = 8,
    LM_ICO_CHECK_CHECKED    = 9,
    LM_ICO_WIRELESS_0       = 10,
    LM_ICO_WIRELESS_1       = 11,
    LM_ICO_WIRELESS_2       = 12,
    LM_ICO_WIRELESS_3       = 13,
    LM_ICO_WIRELESS_4       = 14,
};

extern TFT_STUB tft;

// extern Preferences preferences;

extern int                  currentScreen;
extern int                  currentFont;
extern int                  delayBeforeSleep;
extern int                  delayBeforeLock;
extern int8_t               _signal;
extern int8_t               charge;
extern uint8_t              brightness;
extern uint32_t             wallpaperIndex;
extern ulong                millSleep;
extern volatile int         stateCall;
extern bool                 enableRAMMonitor;
extern bool                 sBarChanged;
extern bool                 isAbleToCall;
extern bool                 isCalling;
extern bool                 isAnswered;
extern bool                 backgroundBusy;
extern bool                 lockFastMode;
extern volatile bool        isScreenLocked;
extern volatile bool        simIsBusy;
extern volatile bool        ongoingCall;
extern volatile bool        simIsUsable;
extern std::vector<Contact> contacts;
extern bool                 mcpexists;
extern bool                 ip5306exists;
extern Contact              examplecontact;

extern NString lastSIMerror;
extern NString currentNumber;
extern NString currentRingtonePath;
extern NString currentNotificationPath;
extern NString currentMailRingtonePath;
extern NString currentWallpaperPath;
extern NString resPath;

extern RenderTarget* currentRenderTarget;
extern RenderTarget* bufferedRenderTarget;

extern iHW* hw;

extern uint8_t* resources;

extern PackageManager pm;
extern PackageStorage PS;

// Function to get the size of an array ... bruh
template <typename T, size_t N> size_t ArraySize(T (&)[N]) { return N; }

extern time_t systemTime;
extern tm     systemTimeInfo;

#ifdef PC
#define TASK std::thread
#elif defined(INC_FREERTOS_H)
#define ENDTASK(x) vTaskDelete(x)
#define TASK       TaskHandle_t
#endif
#ifndef ENDTASK
#define ENDTASK(x) (void)x;
#endif
extern std::vector<TASK> tasks;
#ifdef PC
extern int SDLScale;
#endif
