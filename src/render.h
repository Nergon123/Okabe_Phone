#pragma once
#include "file_include.h"

extern String      resPath;
extern TFT_eSprite screen_buffer;
struct SDImage;
struct mOption;

void changeFont(int ch, bool is_screen_buffer = false,TFT_eSprite &sbuffer = screen_buffer);
void   drawFromSdDownscale(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, int scale, String file_path = resPath);
void   writeText(int x, int y, char *str);
void   drawStatusBar(bool force = false);
void   rendermenu(int &choice, int old_choice);
void   sysError(String reason);
void   drawFromSd(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, bool is_screen_buffer, TFT_eSprite &sbuffer = screen_buffer, String file_path = resPath, bool transp = false, uint16_t tc = 0);
void   drawFromSd(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, bool transp, uint16_t tc);
void   drawFromSd(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, String file_path, bool transp, uint16_t tc = 0);
void   drawFromSd(int x, int y, SDImage sprite);
void   drawFromSd(int x, int y, SDImage sprite, bool is_screen_buffer, TFT_eSprite &sbuffer = screen_buffer);
void   writeCustomFont(int x, int y, String input, int type = 0);
int    listMenu(const String choices[], int icount, bool images, int type, String label, bool forceIcons = false, int findex = 0);
int    listMenu(mOption choices[], int icount, bool lines, int type, String label, bool forceIcons = false, int findex = 0);
void   listMenu_sub(String label, int type, int page, int pages, int y);
int    listMenuNonGraphical(mOption *choices, int icount, String label, int y = 10);
void   spinAnim(int x, int y, int size_x, int size_y, int offset, int spacing = 10);
String SplitString(String text);
int    choiceMenu(const String choices[], int count, bool context);
void   drawCutoutFromSd(SDImage image, int cutout_x, int cutout_y,
                        int cutout_width, int cutout_height,
                        int display_x, int display_y, String file_path = resPath);
int    findCharIndex(String text, int &charIndex, int direction);

String textbox(String title, String content, int ypos, bool onlydraw, bool selected, bool used, int *direction = nullptr, bool onlynumbers = false);
bool   button(String title, int xpos, int ypos, int w, int h, bool selected = false, int *direction = nullptr);
void   sNumberChange(int x, int y, int w, int h, int &val, int min, int max, bool selected = false, int *direction = nullptr, const char *format = "%02d");

void drawPNG(const char *filename, bool _wallpaper = false);
void drawWallpaper();
void progressBar(int val, int max, int y, int h = 8, uint16_t color = TFT_WHITE, bool log = true, bool fast = false);
void findSplitPosition(String text, int charIndex, int &posX, int &posY, int direction = 0);

extern int8_t    _signal;
extern int8_t    charge;
extern const int lastImage;

extern uint8_t *wallpaper;

