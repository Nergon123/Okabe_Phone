#pragma once

#include "../GlobalVariables.h"
#include "../Input/Input.h"
#include "../System/Tasks.h"
int    listMenu(const NString choices[], int icount, bool images, int type, NString label, bool forceIcons = false, int findex = 0);
int    listMenu(mOption choices[], int icount, bool lines, int type, NString label, bool forceIcons = false, int findex = 0);
void   listMenu_sub(NString label, int type, int page, int pages, int y);
int    listMenuNonGraphical(mOption *choices, int icount, NString label, int y = 10);
int    choiceMenu(const NString choices[], int count, bool context);
