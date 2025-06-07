#pragma once
#include "../GlobalVariables.h"
#include "../Screens/Main.h"
#include "../System/DrawGraphics.h"
void drawFromSdDownscale(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, int scale, String file_path = resPath);
void drawCutoutFromSd(SDImage image, int cutout_x, int cutout_y, int cutout_width, int cutout_height, int display_x, int display_y, String file_path = resPath);
void drawImage(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, bool is_screen_buffer, TFT_eSprite &sbuffer = screen_buffer, String file_path = resPath, bool transp = false, uint16_t tc = 0);
void drawImage(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, bool transp, uint16_t tc);
void drawImage(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, String file_path, bool transp, uint16_t tc = 0);
void drawImage(int x, int y, SDImage sprite);
void drawImage(int x, int y, SDImage sprite, bool is_screen_buffer, TFT_eSprite &sbuffer = screen_buffer);
void drawWallpaper();

extern uint8_t *wallpaper;