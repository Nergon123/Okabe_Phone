#pragma once
#include "file_include.h"
extern int currentBrightness;
struct Message;
String sendATCommand(String command, uint32_t timeout = 1000, bool background = false);
void populateContacts();
int getChargeLevel();
int getSignalLevel();
int measureStringHeight(const String &text);
void checkVoiceCall();
bool checkSim();
bool _checkSim();
void parseMessages(Message *&msgs, int &count);
String HEXTOASCII(String hex);
void playAudio(String path);
int GetState();
void printT_S(String str);
void fastMode(bool status);
void setBrightness(int percentage);
int buttonsHelding(bool _idle = true);
void execute_application();
