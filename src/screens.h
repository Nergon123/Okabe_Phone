#ifndef screens_h
#define screens_h
#include "file_include.h"
struct Contact;
void messages();
void contactss();
void e();
void settings();
void MainMenu();
int gallery();
void MainScreen();
void offlineCharging();
void recovery(String message);
void inbox(bool outbox);
String fileBrowser(File dir = SD.open("/"),String format = "*",bool graphical = true);
bool messageActivity(Contact contact, String date = "", String subject = "", String content = "", int index = -1, bool outcoming = false, bool sms = true);
void messageActivityOut(Contact contact, String subject = "", String content = "", bool sms = true);
void downloadFile(const char *url, const char *path);
void WiFiList();
char textInput(int input, bool onlynumbers = false, bool nonl = false);
void incomingCall(Contact contact);
void callActivity(Contact contact);
void editContact(Contact contact);
void ErrorWindow(String reason);
void LockScreen();
bool confirmation(String reason);
void AT_test();
void setTime(time_t *time);
#endif