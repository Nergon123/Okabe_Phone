#pragma once

#include "GlobalVariables.h"
#include "Input/Input.h"
#include "System/Tasks.h"
LM_RET_VALUE listMenu(const NString choices[], int icount, bool images, int type, NString label,
                      bool forceIcons = false, int findex = 0);
void         listMenu_sub(NString label, int type, int page, int pages, int y);
int listMenuNonGraphical(std::vector<mOption> choices, int icount, NString label, int y = 10);
int choiceMenu(const NString choices[], int count, bool context);
LM_RET_VALUE listMenu(std::vector<mOption> choices, int icount, bool images, int type,
                      NString label, bool forceIcons = false, int findex = 0);
