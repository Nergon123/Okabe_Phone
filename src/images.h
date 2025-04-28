#pragma once
#include "file_include.h"


struct SDImage {
    uint32_t address;
    int      w;
    int      h;
    uint16_t tc;
    bool     transp;
    SDImage(uint32_t addr, int width, int height, uint16_t trans_color, bool transparency)
        : address(addr), w(width), h(height), tc(trans_color), transp(transparency) {}
    SDImage() : address(0), w(0), h(0), tc(0), transp(false) {}
    SDImage(uint32_t addr, int width, int height)
        : address(addr), w(width), h(height), tc(0), transp(false) {}
};

// Status Bar
const SDImage STATUSBAR_IMAGE(0x5A708D, 240, 26);

const SDImage BATTERY_IMAGES[] = {
    {0x5AA14D, 33, 26},
    {0x5AA14D + 0x6B4, 33, 26},
    {0x5AA14D + 0x6B4 * 2, 33, 26},
    {0x5AA14D + 0x6B4 * 3, 33, 26}};

const SDImage SIGNAL_IMAGES[5] = {
    {0x5AD47D, 37, 26},
    {0x5ABC1D, 30, 26},
    {0x5ABC1D + 0x6B4, 30, 26},
    {0x5ABC1D + 0x6B4 * 2, 30, 26},
    {0x5ABC1D + 0x6B4 * 3, 30, 26}};

const SDImage NO_SIGNAL_IMAGE(0x5AD47D, 37, 26);

// Wallpapers
const SDImage WALLPAPER_IMAGES_BASE(0xD, 240, 294);
const int     WALLPAPER_IMAGES_MULTIPLIER = 0x22740;

const SDImage MENU_BACKGROUND(0x5ADC01, 240, 294);
// Menu Icons
const SDImage MENU_OFF_ICONS_BASE(0x5D5097, 55, 55);
const int     MENU_OFF_ICONS_MULTIPLIER = 0x17A2;
const SDImage MENU_OFF_ICONS[]          = {
    {0x5D5097, 55, 55, 0x07e0, true},
    {0x5D5097 + 0x17A2, 55, 55, 0x07e0, true},
    {0x5D5097 + 0x17A2 * 2, 55, 55, 0x07e0, true},
    {0x5D5097 + 0x17A2 * 3, 55, 55, 0x07e0, true}};

const SDImage MENU_ON_ICONS[] = {
    {0x5D0341, 49, 49, 0x07e0, true},
    {0x5D1603, 49, 51, 0x07e0, true},
    {0x5D2989, 50, 50, 0x07e0, true},
    {0x5D3D11, 49, 51, 0x07e0, true}};
// Miscellaneous
const SDImage BLUEBAR_IMAGE(0x5DAF1F, 240, 25);

const SDImage BLUEBAR_ICONS[] = {
    {0x5DDDFF, 25, 25},
    {0x5DDDFF + 0x4E2, 25, 25},
    {0x5DDDFF + 0x4E2 * 2, 25, 25}};

const SDImage in_mail[4] = {
    SDImage(0x663E91, 23, 24, 0, false),                     // TIME
    SDImage(0x663E91 + (23 * 24 * 2), 23, 24, 0, false),     //"FROM"
    SDImage(0x663E91 + (23 * 24 * 2 * 2), 23, 24, 0, false), // "TO"
    SDImage(0x663E91 + (23 * 24 * 2 * 3), 23, 24, 0, false), //"SUB"
};

const SDImage mailimg[4] = {
    SDImage(0x662DB1, 18, 21, 0, true),
    SDImage(0x662DB1 + (18 * 21 * 2), 18, 21, 0, true),
    SDImage(0x662DB1 + (18 * 21 * 2 * 2), 18, 21, 0, true),
    SDImage(0x662DB1 + (18 * 21 * 2 * 3), 18, 21, 0, true)};

const SDImage MAIL_HEADER_IMAGE(0x5DECA5, 240, 42);
const SDImage SETTINGS_HEADER_IMAGE(0x5E3B65, 240, 42);
const SDImage WHITE_BOTTOM_IMAGE(0x5E8A25, 240, 128);
const SDImage FULL_SCREEN_NOTIFICATION_IMAGE(0x5F7A25, 240, 134);
const SDImage CONTEXT_MENU_IMAGE(0x607565, 208, 123, 0x07E0, true);
const SDImage BACKGROUND_IMAGE(0x636485, 240, 294);
const SDImage BACKGROUND_IMAGE_CUTTED(0x639365, 240, 269);
const SDImage SUBMENU_BACKGROUND(0x613D45, 240, 294);
const SDImage CALL_ANIMATION_DOTS_BASE(0x658BC5, 7, 7);
const int     CALL_ANIMATION_DOTS_MULTIPLIER = 0x62;
const SDImage VOICE_ONLY_LABEL(0x65D147, 160, 34);
const SDImage PHONE_ICON(0x661AF3, 42, 50, 0xD6BA, true);

const SDImage LIGHTNING_ANIMATION[2] = {
    {0x662B5B, 13, 14, 0xD6BA, true},
    {0x662B5B + 0x1C, 13, 14, 0xD6BA, true}};

const SDImage MAIL_ICONS_BASE(0x662DB1, 18, 21);
const int     MAIL_ICONS_MULTIPLIER = 0x5A8;
const SDImage FOLDER_ICON(0x663981, 18, 18);
const SDImage FILE_ICON(0x663981 + 0x408, 18, 18);
const SDImage INNER_MAIL_ICONS_BASE(0x663E91, 23, 24);
const int     INNER_MAIL_ICONS_MULTIPLIER = 0x708;
const SDImage LOGO_IMAGE(0x665421, 140, 135);
const SDImage WALLPAPER_ICONS_BASE(0x66E7C9, 34, 42);
const int     WALLPAPER_ICONS_MULTIPLIER = 0xB28;
