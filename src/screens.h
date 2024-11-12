#ifndef screens_h
#define screens_h
#include "file_include.h"
void messages();
void contactss();
void e();
bool settings();
void MainMenu();
int gallery();
void MainScreen();
void offlineCharging();
void recovery(String message);
void inbox(bool outbox);
void fileBrowser(File dir = SD.open("/"));
void downloadFile(const char *url, const char *path);
#endif