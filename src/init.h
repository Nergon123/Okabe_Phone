#ifndef INITT_H
#define INITT_H

#include "file_include.h"

struct SDImage {
    uint32_t address;
    int      w;
    int      h;
    uint16_t tc;
    bool     transp;
    SDImage(uint32_t addr, int width, int height, uint16_t trans_color, bool transparency)
        : address(addr), w(width), h(height), tc(trans_color), transp(transparency) {}
    SDImage() : address(0), w(0), h(0), tc(0), transp(false) {}
};

extern SDImage mailimg[4];

struct Contact {
    int    index;
    String phone;
    String name;
    String email;
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
extern BleMouse    blemouse;
extern TFT_eSPI    tft;
extern Preferences preferences;

extern uint          contactCount;
extern int           currentScreen;
extern int           currentFont;
extern int           lastContactIndex;
extern uint32_t      wallpaperIndex;
extern volatile int  DBC_MS;
extern volatile int  stateCall;
extern bool          sBarChanged;
extern bool          isAbleToCall;
extern bool          isCalling;
extern bool          isAnswered;
extern bool          backgroundBusy;
extern volatile bool simIsBusy;
extern volatile bool ongoingCall;
extern volatile bool simIsUsable;

extern Contact contacts[MAX_CONTACTS];
extern Contact examplecontact;
extern String  lastSIMerror;
extern String  currentNumber;
extern String  currentRingtonePath;
extern String  currentNotificationPath;
extern String  currentMailRingtonePath;

template <typename T, size_t N>
size_t ArraySize(T (&)[N]) {
    return N;
}

void loop(void);
void setup();
void idle();
void screens();
void suspendCore(bool suspend);
#endif // INITT_H
