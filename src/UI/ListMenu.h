#pragma once
#include <Arduino.h>
#include "../GlobalVariables.h"
int    listMenu(const String choices[], int icount, bool images, int type, String label, bool forceIcons = false, int findex = 0);
int    listMenu(mOption choices[], int icount, bool lines, int type, String label, bool forceIcons = false, int findex = 0);
void   listMenu_sub(String label, int type, int page, int pages, int y);
int    listMenuNonGraphical(mOption *choices, int icount, String label, int y = 10);
int    choiceMenu(const String choices[], int count, bool context);
