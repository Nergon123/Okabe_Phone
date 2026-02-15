#pragma once
#include "Connectivity/SIM.h"
#include "Defines.h"
#include "GlobalVariables.h"
#include "System/FontManagement.h"

#include "Screens/Main.h"
#include "System/Generic.h"

char     *utf8_encode(uint32_t cp, char* out);
NString  textInput(int input, uint8_t useCharset, int curX, int curY, bool nonl = false, bool dontRedraw = false,
                int *retButton = nullptr);
int       checkButton();
void      numberInput(char first);
void      showText(const char *text, const char *pos);
void      showTextPreview(const char *text, const char *pos);
int       buttonsHelding(bool _idle = true);
void      idle();
