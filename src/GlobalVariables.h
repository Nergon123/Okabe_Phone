#pragma once
#include "Defines.h"

#ifndef PC
#include <Arduino.h>
#include <MCP23017.h>
#include <PNGdec.h>
#include <TFT_eSPI.h>
#endif
#include "Platform/FileSystem/VFS.h"
#include "Platform/Hardware/Hardware.h"
#include "Platform/NString.h"
#include "Platform/Preferences.h"
#include "Platform/ard_esp.h"
#include <vector>
// hmm....
#include "System/ResourceSystem.h"

struct Contact {
    int     index;
    NString phone;
    NString name;
    NString email;
    Contact() : index(-1), phone(""), name(""), email("") {}
    Contact(NString _name, NString _phone, NString _email = "", int _index = -1)
        : index(_index), phone(_phone), name(_name), email(_email) {}
};

// Options for list Menu
struct mOption {
    NString label;
    Image   image;
    uint8_t icon_index;
    mOption(NString label) : label(label), image(Image()), icon_index(0) {};
    mOption(NString label, Image image) : label(label), image(image), icon_index(0) {};
    mOption(NString label, Image image, uint8_t icon_index)
        : label(label), image(image), icon_index(icon_index) {};
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
    NString subject;
    NString content;
    NString date;
    NString longdate;

    operator mOption() const {
        return mOption(date + " " + contact.name, Image(R_LIST_MAIL_ICONS),
                       status == status::NEW ? (uint8_t)0 : (uint8_t)1);
    }

    Message(Contact _contact, NString _subject, NString _content, NString _date, NString _longdate,
            bool _isOutgoing = false, unsigned char _status = status::NEW, int _index = -1)
        : index(_index), status(_status), isOutgoing(_isOutgoing), contact(_contact),
          subject(_subject), content(_content), date(_date), longdate(_longdate) {}

    Message()
        : index(-1), status(status::NEW), isOutgoing(false), contact(Contact()), subject(""),
          content(""), date("00/00"), longdate("00/00/00 00:00") {}
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

// Function to get the size of an array ... bruh
template <typename T, size_t N> size_t ArraySize(T (&)[N]) { return N; }

extern time_t systemTime;
extern tm     systemTimeInfo;

#ifdef PC
extern int SDLScale;
#endif