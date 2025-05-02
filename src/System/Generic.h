#pragma once
#include "../GlobalVariables.h"
#include "../init.h"
int getChargeLevel();
void fastMode(bool status);
void setBrightness(int percentage);
extern int currentBrightness;
void printT_S(String str) ;