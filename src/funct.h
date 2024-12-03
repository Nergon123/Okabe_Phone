#ifndef funct_h
#define funct_h
#include "file_include.h"

String sendATCommand(String command);
void populateContacts();
int getChargeLevel();
int getSignalLevel();
int buttonsHelding();
int measureStringHeight(const String &text, int displayWidth = 240);
void checkVoiceCall();
#endif