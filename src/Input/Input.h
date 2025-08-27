#pragma once
#include "../Defines.h"
#include "../Connectivity/SIM.h"
#include "../GlobalVariables.h"
#include "../System/FontManagement.h"

#include "../Screens/Main.h"
#include "../System/Generic.h"

char textInput(int input, bool onlynumbers = false, bool nonl = false,bool dontRedraw = false,int *retButton = nullptr);
int checkButton();
void numberInput(char first);
void showText(const char *text, int pos);
int buttonsHelding(bool _idle = true);
void idle();