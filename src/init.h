#ifndef INITT_H
#define INITT_H

#include "file_include.h"

#ifdef DEVMODE
extern int charge_d;
extern int signallevel_d;
#endif

struct SDImage
{
    uint32_t address;
    int w;
    int h;
    uint16_t tc;
    bool transp;
   SDImage(uint32_t addr, int width, int height, uint16_t trans_color, bool transparency)
    : address(addr), w(width), h(height), tc(trans_color), transp(transparency) {}
      SDImage() : address(0), w(0), h(0), tc(0), transp(false) {} 
};

struct Contact
{
    int index;
    String phone;
    String name;
    String email;
};
struct Message
{
    int index;
    unsigned char status;
    bool isOutgoing;
    Contact contact;
    String subject;
    String content;
    String date;
};

enum status{
    NEW = 'N',
    REPLIED = 'R',
    READED = 'D'
};
struct STR_DIR{
    String text;
    int direction;
};

struct mOption
{
    String label;
    SDImage icon;
};
enum SCREENS{
    MAINSCREEN,
    MAINMENU,
    MESSAGES,
    CONTACTS,
    SETTINGS,
    E,
  };


extern uint contactCount;
extern IP5306 chrg;
extern TFT_eSPI tft;
extern BleMouse blemouse;
extern uint32_t ima;
extern int currentScreen;
extern Preferences preferences;
extern bool sbchanged;
extern bool isAbleToCall;
extern bool isCalling;
extern volatile bool simIsBusy; 
extern bool ongoingCall;
extern bool isAnswered;
extern SDImage mailimg[4];
extern Contact contacts[MAX_CONTACTS];
extern Contact examplecontact;
extern String currentNumber;
extern int currentFont;



template <typename T, size_t N>
size_t ArraySize(T (&)[N])
{
    return N;
}

void loop(void);
void setup();
void idle();
void screens();
void suspendCore(bool suspend);
#endif // INITT_H
