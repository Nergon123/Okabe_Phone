#pragma once
#include "../Input/Input.h"
#include "GlobalVariables.h"
#include "../System/FontManagement.h"
#include "../System/ResourceSystem.h"

enum UITypes { UI_BUTTON, UI_INPUT, UI_SWITCHNUMBERS };
struct UIElement {
    int         type;
    int         x, y;
    int         w, h;
    NString      title;
    NString     *input;
    int        *value;
    bool       *bvalue;
    int        *selected;
    bool        usable;
    bool        onlynumbers;
    const char *format;
    int         min, max;
    bool        isExitButton;
    void (*callback)(void *);
};

bool   button(NString title, int xpos, int ypos, int w, int h, bool selected = false, int *direction = nullptr);
void   sNumberChange(int x, int y, int w, int h, int &val, int min, int max, bool selected = false, int *direction = nullptr, const char *format = "%02d");
NString InputField(NString title, NString content, int ypos, bool onlydraw, bool selected, bool used, int *direction = nullptr, bool onlynumbers = false);
void   spinAnim(int x, int y, int size_x, int size_y, int offset, int spacing = 10);
void   progressBar(int val, int max, int y = 250, int h = 8, uint16_t color = TFT_WHITE, bool log = true, bool fast = false);
void   bootText(NString text, int x = -1, int y = 260, int w = 240, int h = 30);
void   sysError(NString reason);
