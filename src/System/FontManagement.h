#pragma once
#include "../GlobalVariables.h"
#include "../System/DrawGraphics.h"
void changeFont(int ch, bool is_screen_buffer = false,TFT_eSprite &sbuffer = screen_buffer);
void   writeCustomFont(int x, int y, String input, int type = 0);