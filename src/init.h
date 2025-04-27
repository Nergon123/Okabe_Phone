#pragma once

#include "file_include.h"



extern SDImage mailimg[4];

struct Contact {
    int    index;
    String phone;
    String name;
    String email;
    Contact() : index(-1), phone(""), name(""), email("") {}
    Contact(String _name, String _phone, String _email = "", int _index = -1) : index(_index), phone(_phone), name(_name), email(_email) {}
};

struct mOption {

    String  label;
    SDImage icon;
};

enum status {

    NEW     = 'N',
    REPLIED = 'R',
    READED  = 'D'
};

struct Message {
    int           index;
    unsigned char status;
    bool          isOutgoing;
    Contact       contact;
    String        subject;
    String        content;
    String        date;
    String        longdate;

    operator mOption() const {
        return mOption{date + " " + contact.name, status == status::NEW ? mailimg[0] : mailimg[1]};
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

extern MCP23017    mcp;
extern IP5306      chrg;
extern TFT_eSPI    tft;
extern TFT_eSprite screen_buffer;
extern Preferences preferences;
extern PNG         png;

extern uint          contactCount;
extern int           currentScreen;
extern int           currentFont;
extern int           lastContactIndex;
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

extern uint8_t *resources;

extern bool isSPIFFS;
template <typename T, size_t N>
size_t ArraySize(T (&)[N]) {
    return N;
}

extern time_t systemTime;
extern tm systemTimeInfo;

void SetUpTime();
void SaveTime(time_t time);
void loop(void);
void setup();
void idle();
void screens();
void suspendCore(bool suspend);
bool initSDCard(bool fast);
void loadResource(ulong address, String resourcefile, uint8_t **_resources,int w=0,int h=0);
