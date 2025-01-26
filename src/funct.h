#ifndef funct_h
#define funct_h
#include "file_include.h"
struct Message;
String sendATCommand(String command, uint32_t timeout = 1000, bool background = false);
void populateContacts();
int getChargeLevel();
int getSignalLevel();
int buttonsHelding();
int measureStringHeight(const String &text);
void checkVoiceCall();
bool checkSim();
bool _checkSim();
void parseMessages(Message *&msgs, int &count);
String HEXTOASCII(String hex);
void playAudio(String path);
int GetState();
#endif