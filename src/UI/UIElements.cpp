#include "UIElements.h"
#include <functional>
#include "../Platform/ESPPlatform.h"
const uint16_t clr_selected   = TFT_RED;
const uint16_t clr_normal     = TFT_BLACK;
const uint16_t clr_disabled   = TFT_LIGHTGREY;
const uint16_t clr_background = TFT_WHITE;

// void UIElementsLoop(UIElement *elements, size_t size, bool *exit) {
//     // TODO
//     if (!elements) {
//         ESP_LOGE("UI", "elements is nullptr");
//         return;
//     }
//     if (!exit) {
//         ESP_LOGE("UI", "exit is nullptr");
//         return;
//     }
//     while (!*exit) {
//         int direction     = -1;
//         int cur_selection = 0;
//         for (int i = 0; i < size; i++) {
//             UIElement el = elements[i];

//             switch (el.type) {
//             case UI_BUTTON:
//                 *el.bvalue = button(el.title, el.x, el.y, el.w, el.h, i == cur_selection,
//                                     &direction, el.usable, el.callback);
//                 if (el.isExitButton && *el.bvalue) {
//                     *exit = true;
//                     return;
//                 }
//                 break;
//             case UI_INPUT:
//                 if (el.input == nullptr) { el.input = new NString(); }
//                 *el.input = InputField(el.title, *el.input, el.y, false, i == cur_selection,
//                                        i == cur_selection, &direction, el.onlynumbers, el.usable,
//                                        el.callback);
//                 break;
//             case UI_SWITCHNUMBERS:
//                 sNumberChange(el.x, el.y, el.w, el.h, *el.value, el.min, el.max,
//                               i == cur_selection, &direction, el.format, el.usable, el.callback);
//                 break;
//             }
//             if (direction == LEFT) { direction = UP; }
//             if (direction == RIGHT) { direction = DOWN; }
//             switch (direction) {
//             case UP:
//                 cur_selection++;
//                 if (cur_selection >= size) { cur_selection = 0; }
//                 break;
//             case DOWN:
//                 cur_selection--;
//                 if (cur_selection < 0) { cur_selection = size - 1; }
//                 break;
//             }
//         }
//     }
// }

// ## UI Button
//  Part of local "UI Kit"
//  @param title: Button title
//  @param xpos: X position on the screen
//  @param ypos: Y position on the screen
//  @param w: Button width
//  @param h: Button height
//  @param selected: Boolean indicating if the button is selected
//  @param direction: Pointer to the direction variable
//  @return: Boolean indicating if the button was pressed
bool button(NString title, int xpos, int ypos, int w, int h, bool selected, int *direction,
            bool usable, void (*callback)(void *)) {

    tft.fillRect(xpos, ypos, w, h, clr_background);
    tft.drawRect(xpos, ypos, w, h, clr_normal);
    changeFont(1);
    if (selected) {
        tft.setTextColor(clr_selected);
        tft.drawRect(xpos, ypos, w, h, clr_selected);
    }
    if (!usable) {
        tft.setTextColor(clr_disabled);
        tft.drawRect(xpos, ypos, w, h, clr_disabled);
    }

    int x = (w - tft.textWidth(title.c_str())) / 2;
    int y = h - ((h - tft.fontHeight()));

    tft.setCursor(xpos + x, ypos + y);
    tft.print(title.c_str());
    if (!usable) { return false; }
    bool exit = false;

    if (selected) {
        while (!exit) {
            *direction = buttonsHelding();
            if (*direction == SELECT) {
                if (callback != nullptr) { callback(nullptr); }
                return true;
            }
            else if (*direction != -1) { exit = true; }
        }
    }

    tft.setTextColor(clr_normal);
    tft.fillRect(xpos, ypos, w, h, clr_background);
    tft.drawRect(xpos, ypos, w, h, clr_normal);
    tft.setCursor(xpos + x, ypos + y);
    tft.print(title);

    return false;
}

// Input field for numbers
// @param x: X position on the screen
// @param y: Y position on the screen
// @param w: Width of the input field
// @param h: Height of the input field
// @param val: Reference to the value to be changed
// @param min: Minimum value
// @param max: Maximum value
// @param selected: Boolean indicating if the input field is selected
// @param direction: Pointer to the direction variable
// @param format: Format string for the value
void sNumberChange(int x, int y, int w, int h, int &val, int min, int max, bool selected,
                   int *direction, const char *format, bool usable, void (*callback)(void *)) {
    // calculation of triangle corners
    int fp  = w / 2;
    int st  = h / 5;
    int stf = w / 5;

    int tx1 = x + fp - stf, tx2 = x + fp, tx3 = x + fp + stf;
    int ty1 = y - st, ty2 = y - st * 2, ty3 = y - st;
    int ty11 = y + h + st, ty21 = y + h + st * 2, ty31 = y + h + st;

    tft.resetViewport();

    tft.fillTriangle(tx1, ty1, tx2, ty2, tx3, ty3, selected ? clr_selected : clr_normal);
    tft.fillTriangle(tx1, ty11, tx2, ty21, tx3, ty31, selected ? clr_selected : clr_normal);

    changeFont(1);
    tft.setTextSize(1);
    tft.setTextColor(clr_normal);
    tft.setTextWrap(false);
    tft.setViewport(x, y, w, h);
    if (selected) { tft.setTextColor(clr_selected); }
    else { tft.setTextColor(clr_normal); }
    char text[64];
    snprintf(text, sizeof(text), format, val);
    int height = tft.fontHeight();
    tft.setTextSize(h / height);

    height                        = tft.fontHeight();
    int                   width   = tft.textWidth(text);
    std::function<void()> DrawBox = [&]() {
        snprintf(text, sizeof(text), format, val);
        tft.setCursor(w / 2 - width / 2, h / 2 + height / 4);
        tft.drawRect(0, 0, w, h, selected ? clr_selected : clr_normal);
        tft.fillRect(1, 1, w - 2, h - 2, clr_background);
        tft.print(text);
    };
    DrawBox();
    bool exit = false;

    while (!exit && selected) {
        *direction = buttonsHelding();
        switch (*direction) {
        case DOWN:
            val--;
            if (val < min) { val = max; }
            DrawBox();
            break;
        case UP:
            val++;
            if (val > max) { val = min; }
            DrawBox();
            break;
        case LEFT: exit = true; break;
        case RIGHT: exit = true; break;
        }
    }
    tft.resetViewport();
}

// ## UI Textbox
//  Part of local "UI Kit"
//  Textbox for text input
//  @param title: Title of the textbox
//  @param content: Content of the textbox
//  @param ypos: Y position on the screen
//  @param onlydraw: Boolean indicating if the textbox should be drawn and return
//  @param selected: Boolean indicating if the textbox is selected (draw borders red)
//  @param used: Boolean indicating if the textbox is used
//  @param direction: Pointer to the direction variable
//  @param onlynumbers: Boolean indicating if only numbers are allowed
//  @return: String containing the input
NString InputField(NString title, NString content, int ypos, bool onlydraw, bool selected, bool used,
                  int *direction, bool onlynumbers, bool usable, void (*callback)(void *)) {

    content.trim();
    if (used) { selected = true; }
    tft.resetViewport();
    tft.setTextColor(clr_normal);
    tft.setTextWrap(false);
    tft.setCursor(5, ypos - 5);
    changeFont(1);
    tft.setTextSize(1);

    if (selected) { tft.setTextColor(clr_selected); }
    else { tft.setTextColor(clr_normal); }

    tft.print(title);
    tft.setTextColor(clr_normal);
    changeFont(3);

    int cursorPos = content.length();
    int yoff      = tft.fontHeight();
    int xpos      = 5;
    int width     = 235 - xpos;
    int height    = 25;
    int dypos     = 0;
    int c_offset  = 0;

    tft.setViewport(xpos, ypos, width, height);
    xpos = 0;
    tft.setCursor(xpos + 5, dypos + yoff);

    int length = tft.textWidth(content.substring(0, cursorPos)) + 15;

    if (length > width) { c_offset = width - length; }

    if (selected) { tft.setCursor(xpos + 5 + c_offset, dypos + yoff); }

    tft.fillRect(xpos, dypos, width, height, clr_background);
    tft.drawRect(xpos, dypos, width, height, clr_normal);

    if (selected) { tft.drawRect(xpos, dypos, width, height, clr_selected); }

    tft.print(content);

    bool exit = false;
    int  c    = -1;

    if (!onlydraw) {
        if (used) {
            length = tft.textWidth(content.substring(0, cursorPos)) + 15;

            if (length > width) { c_offset = width - length; }

            tft.fillRect(tft.textWidth(content.substring(0, cursorPos)) + c_offset + 5, 3, CWIDTH,
                         height - 6, clr_normal);

            tft.drawRect(xpos, ypos, width, height, clr_selected);

            while (!exit) {

                while (c == -1) { c = buttonsHelding(); }
                if (c != -1) {
                    if (c >= '0' && c <= '9') {

                        // Serial.println("C_OFFSET:" + NString(c_offset));

                        char TI = textInput(c, onlynumbers, true);
                        if (TI != '\0') {
                            if (TI != '\n') {
                                if (TI != '\b') {
                                    content = content.substring(0, cursorPos) + TI +
                                              content.substring(cursorPos, content.length());
                                    cursorPos++;
                                }
                                else {
                                    content = content.substring(0, cursorPos - 1) +
                                              content.substring(cursorPos, content.length());
                                    cursorPos--;
                                }
                            }
                        }
                        length = tft.textWidth(content.substring(0, cursorPos)) + 15;
                        if (length > width) { c_offset = width - length; }
                        tft.fillRect(xpos, dypos, width, height, clr_background);
                        tft.drawRect(xpos, dypos, width, height, clr_selected);
                        tft.setCursor(xpos + 5 + c_offset, dypos + yoff);
                        tft.println(content);
                        tft.fillRect(tft.textWidth(content.substring(0, cursorPos)) + c_offset + 5,
                                     3, CWIDTH, height - 6, clr_normal);
                        c = buttonsHelding();
                    }
                    else {
                        switch (c) {
                        case LEFT:
                            if (cursorPos > 0) {

                                cursorPos--;
                                length = tft.textWidth(content.substring(0, cursorPos)) + 15;
                                if (length > width) { c_offset = width - length; }
                                tft.fillRect(xpos, dypos, width, height, clr_background);
                                tft.drawRect(xpos, dypos, width, height, clr_selected);
                                tft.setCursor(xpos + 5 + c_offset, dypos + yoff);
                                tft.println(content);
                                tft.fillRect(tft.textWidth(content.substring(0, cursorPos)) +
                                                 c_offset + 5,
                                             3, CWIDTH, height - 6, clr_normal);
                                tft.fillRect(tft.textWidth(content.substring(0, cursorPos + 1)) +
                                                 c_offset + 5,
                                             3, CWIDTH, height - 6, clr_background);
                            }
                            break;
                        case RIGHT:
                            if (cursorPos < (int)(content.length())) {
                                cursorPos++;
                                length = tft.textWidth(content.substring(0, cursorPos)) + 15;
                                if (length > width) { c_offset = width - length; }
                                tft.fillRect(xpos, dypos, width, height, clr_background);
                                tft.drawRect(xpos, dypos, width, height, clr_selected);
                                tft.setCursor(xpos + 5 + c_offset, dypos + yoff);
                                tft.println(content);
                                tft.fillRect(tft.textWidth(content.substring(0, cursorPos - 1)) +
                                                 c_offset + 5,
                                             3, CWIDTH, height - 6, clr_normal);
                                tft.fillRect(tft.textWidth(content.substring(0, cursorPos)) +
                                                 c_offset + 5,
                                             3, CWIDTH, height - 6, clr_background);
                            }
                            break;

                        case UP:
                            *direction = UP;
                            exit       = true;
                            break;

                        case DOWN:
                            *direction = DOWN;
                            exit       = true;
                            break;
                        default: *direction = BACK; break;
                        }
                        c = buttonsHelding();
                    }
                }
            }
        }
    }

    tft.fillRect(tft.textWidth(content.substring(0, cursorPos)) + c_offset + 5, 3, CWIDTH,
                 height - 6, clr_background);
    tft.fillRect(xpos, dypos, width, height, clr_background);
    tft.drawRect(xpos, dypos, width, height, clr_normal);
    tft.setCursor(xpos + 5, dypos + yoff);
    tft.println(content);
    tft.resetViewport();
    tft.setCursor(5, ypos - 5);
    changeFont(1);
    tft.setTextSize(1);
    tft.setTextColor(clr_normal);
    tft.print(title);
    content.trim();
    return content;
}

// Animation of spinning circles in rectangle, used in outgoing call
// @param x: X position on the screen
// @param y: Y position on the screen
// @param size_x: Width of the rectangle
// @param size_y: Height of the rectangle
// @param offset: Offset for the animation
// @param spacing: Spacing between the circles
void spinAnim(int x, int y, int size_x, int size_y, int offset, int spacing) {
    // Get image data and allocate buffer
    ImageData img           = res.GetImageDataByID(R_CALL_ANIM_DOTS);
    int       max_count     = (2 * size_x) + (2 * (size_y - 1));
    int       printed_count = 0;
    int       xt = 0, yt = 0;
    bool      draw = true;

    while (printed_count < max_count) {
        for (int j = offset; j >= 0 && printed_count < max_count; j--) {
            if (draw) { res.DrawImage(R_CALL_ANIM_DOTS, j % img.count, {x + xt, y + yt}); }

            // Update position in spin path
            if (printed_count < size_x) { xt += spacing; }
            else if (printed_count < size_x + size_y - 1) { yt += spacing; }
            else if (printed_count < (2 * size_x) + size_y - 1) { xt -= spacing; }
            else {
                yt -= spacing;
                if (yt <= -((size_y - 1) * spacing)) { draw = false; }
            }

            printed_count++;
        }

        offset = 7; // Reset offset after each full circle iteration
    }
}

int lastpercentage;
// progress bar that used on boot screen
// @param val: Current value
// @param max: Maximum value
// @param y: Y position on the screen
// @param h: Height of the progress bar
// @param color: Color of the progress bar
// @param log: Boolean indicating if the progress bar is in log mode
// @param fast: Boolean indicating if the progress bar should be fast or smooth
void progressBar(int val, int max, int y, int h, uint16_t color, bool log, bool fast) {

    int percentage = (val * 100) / max;
    if (lastpercentage > percentage) { lastpercentage = percentage; }
    if (!log) {
        //"for loop" and delay for smooth transition
        tft.drawRect(69, y, 100, h, color);
        for (int i = lastpercentage; i <= percentage; i++) {
            tft.fillRect(69, y, i, h, color);
            if (!fast) { delay(5); }
        }
    }
    else {
#ifndef LOG
        tft.drawRect(69, y, 100, h, color);
        for (int i = lastpercentage; i <= percentage; i++) {
            tft.fillRect(69, y, i, h, color);
            if (!fast) { delay(5); }
        }
#endif
    }
    lastpercentage = percentage;
}

void bootText(NString text, int x, int y, int w, int h) {
    if (lastpercentage == 100) { return; }
    tft.fillRect(x, y, w, h, TFT_BLACK);
    tft.setTextFont(0);
    tft.setTextSize(1);
    if (x < 0) { x = 120 - (tft.textWidth(text.c_str()) / 2); }
    tft.setViewport(0, y, w, h);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(x, 0);
    tft.print(text.c_str());
    tft.resetViewport();
}

// ## Critical system error
//  This function is called when a critical error occurs in the system.
// @param reason String containing the reason for the error
void sysError(NString reason) {
    tft.fillScreen(0x0000);
    tft.setCursor(10, 40);
    tft.setTextFont(1);
    tft.setTextSize(4);
    tft.setTextColor(0xF001); // FOOL :3
    tft.println("==ERROR==");
    tft.setTextColor(0xFFFF);
    tft.setTextSize(1);
    tft.println(NString(
        "\n\n\nThere a problem with your device\n\nTechnical details:\n\n\nReason:" + reason +
        "\n\n\n\n\n" REPOSITORY_LINK "\n\n").c_str());

    tft.println("Press any button to restart\nor reset button to reset the device");
    ESP_LOGE("ERROR", "%s", reason.c_str());
    while (buttonsHelding(false) == -1);
    #ifndef PC
    ESP.restart();
    #endif
    for (;;);
}
