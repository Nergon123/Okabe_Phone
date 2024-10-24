
#ifndef RENDER_H
#define RENDER_H
#include "file_include.h"

void drawFromSdTrans(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, uint16_t tc, File file);
void changeFont(int ch);
void drawFromSdDownscale(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, int scale, File file);
void writeText(int x, int y, char *str);
void drawStatusBar();
void rendermenu(int choice, bool right);
void blueScreen(char *reason);
void drawFromSd(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, File file);
int listMenu(const String choices[], int icount, bool images, int type, String label);
void listMenu_sub(String label, int type, int page, int pages);
void spinAnim(int x,int y,int size_x,int size_y,int offset);
int choiceMenu(const String choices[], int count, bool context);
#endif