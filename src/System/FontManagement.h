#pragma once
#include "GlobalVariables.h"
void changeFont(int ch);
void writeCustomFont(int x, int y, NString input, int type = 0,bool background=false,uint16_t bgColor= TFT_BLACK);