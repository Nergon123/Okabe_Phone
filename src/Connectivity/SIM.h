#pragma once
#include "../GlobalVariables.h"
#include "../Input/Input.h"

#include "../System/Generic.h"
#include "../System/TextManipulation.h"
#include "../UI/UIElements.h"

void   AT_test();
NString sendATCommand(NString command, uint32_t timeout = 1000, bool background = false);
NString getATvalue(NString command, bool background = false);
bool   checkSim();
bool   _checkSim();
void   initSim();
int    getSignalLevel();
void   populateContacts();
void   checkVoiceCall();
int GetState();