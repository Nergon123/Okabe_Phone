#pragma once
#include "../GlobalVariables.h"
#include "../init.h"
int getChargeLevel();
void fastMode(bool status);
void setBrightness(uint8_t percentage);
extern uint8_t currentBrightness;
