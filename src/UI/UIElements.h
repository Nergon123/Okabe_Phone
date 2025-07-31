#pragma once
#include "GlobalVariables.h"
#include "../Input/Input.h"
#include "../System/DrawGraphics.h"
#include "../System/FontManagement.h"
#include "../System/ResourceSystem.h"
bool   button(String title, int xpos, int ypos, int w, int h, bool selected = false, int *direction = nullptr);
void   sNumberChange(int x, int y, int w, int h, int &val, int min, int max, bool selected = false, int *direction = nullptr, const char *format = "%02d");
String InputField(String title, String content, int ypos, bool onlydraw, bool selected, bool used, int *direction = nullptr, bool onlynumbers = false);
void   spinAnim(int x, int y, int size_x, int size_y, int offset, int spacing = 10);
void   progressBar(int val, int max, int y, int h = 8, uint16_t color = TFT_WHITE, bool log = true, bool fast = false);
void   sysError(String reason);
