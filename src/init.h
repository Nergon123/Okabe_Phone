#ifndef INITT_H
#define INITT_H

#include "file_include.h"

struct Contact
{
    int index;
    String number;
    String name;
};
struct Message
{
    int index;
    Contact contact;
    String subject;
    String content;
    String date;
};

extern int contactCount;
extern IP5306 chrg;
extern TFT_eSPI tft;
extern uint32_t ima;

extern Preferences preferences;

extern Contact contacts[MAX_CONTACTS];

const String contmenu[] = {
    "Call",
    "Outgoing",
    "Edit",
    "Create",
    "Delete"
};

const String wallnames[] = {
    "Wallpaper 1",
    "Wallpaper 2",
    "Wallpaper 3",
    "IBN5100",
    "Red jelly",
    "The head of doll",
    "Mayuri jelly",
    "Fatty Gero Froggy",
    "Burning Gero Froggy",
    "Upa",
    "Metal Upa",
    "Seira",
    "Seira After awaking",
    "Gero Froggy",
    "Cat Gero Froggy",
    "Cow Gero Froggy",
    "FES",
    "Gero Froggies",
    "Calico Gero Froggies",
    "Gold Upa",
    "FES2",
    "Erin 1",
    "Erin 2",
    "Orgel Sisters",
    "Mayuri",
    "Kurisu",
    "Moeka",
    "Luka",
    "Faris",
    "Suzuha",
    "UNCONFIRMED",
    "Popping steiner",
    "Wallpaper 4",
    "NukariyaIce",
    "MayQueen",
    "Upa ♪",
    "Wallpaper 5",
    "Rabikuro",
    "Wallpaper 6",
    "Space Froggies",
    "Wallpaper 7",
    "Nae"
};


template <typename T, size_t N>
size_t ArraySize(T (&)[N])
{
    return N;
}

void loop(void);
void setup();


#endif // INITT_H
