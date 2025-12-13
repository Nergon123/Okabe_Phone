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