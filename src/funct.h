#ifndef funct_h
#define funct_h
#include "file_include.h"
struct Message;
String sendATCommand(String command, uint32_t timeout = 1000, bool background = false);
void populateContacts();
int getChargeLevel();
int getSignalLevel();
int buttonsHelding();
int measureStringHeight(const String &text, int displayWidth = 240, int addLines = 0);
void checkVoiceCall();
void parseMessages(Message *&msgs, int &count);
#endif