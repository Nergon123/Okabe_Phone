#pragma once

#include "../GlobalVariables.h"
#include "../Input/Input.h"
#include "../System/Tasks.h"
int  listMenu(const NString choices[], int icount, bool images, int type, NString label,
              bool forceIcons = false, int findex = 0);
void listMenu_sub(NString label, int type, int page, int pages, int y);
int  listMenuNonGraphical(std::vector<mOption> choices, int icount, NString label, int y = 10);
int  choiceMenu(const NString choices[], int count, bool context);
int  listMenu(std::vector<mOption> choices, int icount, bool images, int type, NString label,
              bool forceIcons = false, int findex = 0);

enum LM_TYPE {
    LM_MESSAGES = 0,
    LM_CONTACTS = 1,
    LM_SETTINGS = 2,
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
};