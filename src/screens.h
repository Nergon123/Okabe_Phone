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
void fileBrowser(File dir = SD.open("/"));
void messageActivity(Contact contact,String date,String subject,String content, bool outcoming = false);
void downloadFile(const char *url, const char *path);
void WiFiList();
char textInput();
#endif