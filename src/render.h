
#ifndef RENDER_H
#define RENDER_H
#include "file_include.h"
struct SDImage;
struct mOption;
void changeFont(int ch);
void drawFromSdDownscale(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, int scale, File file = SD.open("/FIRMWARE/IMAGES.SG", FILE_READ));
void writeText(int x, int y, char *str);
void drawStatusBar();
void rendermenu(int choice, bool right);
void sysError(const char *reason);
void drawFromSd(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, File file = SD.open("/FIRMWARE/IMAGES.SG", FILE_READ),bool transp =false,uint16_t tc = 0);
void drawFromSd(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y,bool transp,uint16_t tc = 0);
void drawFromSd(int x, int y, SDImage sprite);
void writeCustomFont(int x, int y, String input,int type = 0);
int listMenu(const String choices[], int icount, bool images, int type, String label);
int listMenu(mOption choices[], int icount, bool images, int type, String label);
void listMenu_sub(String label, int type, int page, int pages);
void spinAnim(int x,int y,int size_x,int size_y,int offset,int spacing = 10);

int choiceMenu(const String choices[], int count, bool context);
#endif