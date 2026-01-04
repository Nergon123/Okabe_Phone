#pragma once
#include "GlobalVariables.h"
#include "Input/Input.h"
#include "System/FontManagement.h"
#include "System/ResourceSystem.h"
#include <UI/ListMenu.h>
struct FIELD {
    const NString name;
    NString      &resultstr;
    NString       notConfirmed;
    bool          OnlyNumbers;
    FIELD(NString name, NString &resultstr, bool OnlyNumbers)
        : name(name), resultstr(resultstr), OnlyNumbers(OnlyNumbers) {};
};
bool InputFieldS(NString title, std::vector<FIELD> fields, int type = LM_SETTINGS,
                 int selected = 0, NString confirmbtn = "Confirm", NString cancelbtn = "Cancel");
bool button(NString title, int xpos, int ypos, int w, int h, bool selected = false,
            int *direction = nullptr);
void sNumberChange(int x, int y, int w, int h, int &val, int min, int max, bool selected = false,
                   int *direction = nullptr, const char *format = "%02d");
NString InputField(NString title, NString content, int ypos, bool onlydraw, bool selected,
                   bool used, int *direction = nullptr, bool onlynumbers = false);
void    spinAnim(int x, int y, int size_x, int size_y, int offset, int spacing = 10);
void    progressBar(int val, int max, int y = 250, int h = 8, uint16_t color = TFT_WHITE,
                    bool log = true, bool fast = false);
void    bootText(NString text, int x = -1, int y = 260, int w = 240, int h = 30);
void    sysError(NString reason);
void    sysWarn(NString reason);