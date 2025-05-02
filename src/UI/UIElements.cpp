#include "UIElements.h"


// ## UI Button
//  Part of local "UI Kit"

bool button(String title, int xpos, int ypos, int w, int h, bool selected, int *direction) {
    tft.fillRect(xpos, ypos, w, h, 0xFFFF);
    tft.drawRect(xpos, ypos, w, h, 0);
    changeFont(1);
    if (selected) {
        tft.setTextColor(TFT_RED);
        tft.drawRect(xpos, ypos, w, h, 0);
        tft.drawRect(xpos, ypos, w, h, TFT_RED);
    }

    int x = (w - tft.textWidth(title)) / 2;
    int y = h - ((h - tft.fontHeight()));

    // Serial.println("X:" + String(x));
    // Serial.println("Y:" + String(y));

    tft.setCursor(xpos + x, ypos + y);
    tft.print(title);

    bool exit = false;

    if (selected)
        while (!exit) {
            *direction = buttonsHelding();
            if (*direction == SELECT) {
                return true;
            } else if (*direction != -1) {
                exit = true;
            }
        }

    tft.setTextColor(0);
    tft.fillRect(xpos, ypos, w, h, 0xFFFF);
    tft.drawRect(xpos, ypos, w, h, 0);
    tft.setCursor(xpos + x, ypos + y);
    tft.print(title);

    return false;
}

// Input field for numbers
void sNumberChange(int x, int y, int w, int h, int &val, int min, int max, bool selected, int *direction, const char *format) {

    int fp  = w / 2;
    int st  = h / 5;
    int stf = w / 5;

    int tx1 = x + fp - stf, tx2 = x + fp, tx3 = x + fp + stf;
    int ty1 = y - st, ty2 = y - st * 2, ty3 = y - st;
    int ty11 = y + h + st, ty21 = y + h + st * 2, ty31 = y + h + st;

    tft.resetViewport();

    tft.fillTriangle(tx1, ty1, tx2, ty2, tx3, ty3, selected ? TFT_RED : TFT_BLACK);
    tft.fillTriangle(tx1, ty11, tx2, ty21, tx3, ty31, selected ? TFT_RED : TFT_BLACK);

    changeFont(1);
    tft.setTextSize(1);
    tft.setTextColor(0);
    tft.setTextWrap(false);
    tft.setViewport(x, y, w, h);
    if (selected) {
        tft.setTextColor(TFT_RED);
    } else {
        tft.setTextColor(0);
    }
    char text[64];
    snprintf(text, sizeof(text), format, val);
    int height = tft.fontHeight();
    tft.setTextSize(h / height);

    height                        = tft.fontHeight();
    int                   width   = tft.textWidth(text);
    std::function<void()> DrawBox = [&]() {
        snprintf(text, sizeof(text), format, val);
        tft.setCursor(w / 2 - width / 2, h / 2 + height / 4);
        tft.drawRect(0, 0, w, h, selected ? TFT_RED : TFT_BLACK);
        tft.fillRect(1, 1, w - 2, h - 2, TFT_WHITE);
        tft.print(text);
    };
    DrawBox();
    bool exit = false;

    while (!exit && selected) {
        *direction = buttonsHelding();
        switch (*direction) {
        case DOWN:
            val--;
            if (val < min)
                val = max;
            DrawBox();
            break;
        case UP:
            val++;
            if (val > max)
                val = min;
            DrawBox();
            break;
        case LEFT:
            exit = true;
            break;
        case RIGHT:
            exit = true;
            break;
        }
    }
    tft.resetViewport();
}

// ## UI Textbox
//  Part of local "UI Kit"
//  Textbox for text input
String InputField(String title, String content, int ypos, bool onlydraw, bool selected, bool used, int *direction, bool onlynumbers) {

    content.trim();
    if (used)
        selected = true;
    tft.resetViewport();
    tft.setTextColor(0);
    tft.setTextWrap(false);
    tft.setCursor(5, ypos - 5);
    changeFont(1);
    tft.setTextSize(1);

    if (selected)
        tft.setTextColor(TFT_RED);
    else
        tft.setTextColor(0);

    tft.print(title);
    tft.setTextColor(0);
    changeFont(3);

    int cursorPos = content.length();
    int yoff      = tft.fontHeight();
    // int charWidth = tft.textWidth("D") + 1;
    int xpos   = 5;
    int width  = 235 - xpos;
    int height = 25;

    int dypos    = 0;
    int c_offset = 0;

    tft.setViewport(xpos, ypos, width, height);
    xpos = 0;
    tft.setCursor(xpos + 5, dypos + yoff);

    int length = tft.textWidth(content.substring(0, cursorPos)) + 15;

    if (length > width)
        c_offset = width - length;

    if (selected)
        tft.setCursor(xpos + 5 + c_offset, dypos + yoff);

    tft.fillRect(xpos, dypos, width, height, 0xFFFF);
    tft.drawRect(xpos, dypos, width, height, 0);

    if (selected) {
        tft.drawRect(xpos, dypos, width, height, 0xF800);
    }

    tft.print(content);

    bool exit = false;
    int  c    = -1;

    if (!onlydraw)
        if (used) {
            length = tft.textWidth(content.substring(0, cursorPos)) + 15;
            if (length > width)
                c_offset = width - length;

            tft.fillRect(tft.textWidth(content.substring(0, cursorPos)) + c_offset + 5, 3, CWIDTH, height - 6, 0);

            tft.drawRect(xpos, ypos, width, height, 0xF800);

            while (!exit) {

                while (c == -1)
                    c = buttonsHelding();
                if (c != -1) {
                    if (c >= '0' && c <= '9') {

                        // Serial.println("C_OFFSET:" + String(c_offset));

                        char TI = textInput(c, onlynumbers, true);
                        if (TI != '\0')
                            if (TI != '\n') {
                                if (TI != '\b') {
                                    content = content.substring(0, cursorPos) + TI + content.substring(cursorPos, content.length());
                                    cursorPos++;
                                } else {
                                    content = content.substring(0, cursorPos - 1) + content.substring(cursorPos, content.length());
                                    cursorPos--;
                                }
                            }
                        length = tft.textWidth(content.substring(0, cursorPos)) + 15;
                        if (length > width)
                            c_offset = width - length;
                        tft.fillRect(xpos, dypos, width, height, 0xFFFF);
                        tft.drawRect(xpos, dypos, width, height, 0xF800);
                        tft.setCursor(xpos + 5 + c_offset, dypos + yoff);
                        tft.println(content);
                        tft.fillRect(tft.textWidth(content.substring(0, cursorPos)) + c_offset + 5, 3, CWIDTH, height - 6, 0);
                        c = buttonsHelding();
                    } else {
                        switch (c) {
                        case LEFT:
                            if (cursorPos > 0) {

                                cursorPos--;
                                length = tft.textWidth(content.substring(0, cursorPos)) + 15;
                                if (length > width)
                                    c_offset = width - length;
                                tft.fillRect(xpos, dypos, width, height, 0xFFFF);
                                tft.drawRect(xpos, dypos, width, height, 0xF800);
                                tft.setCursor(xpos + 5 + c_offset, dypos + yoff);
                                tft.println(content);
                                tft.fillRect(tft.textWidth(content.substring(0, cursorPos)) + c_offset + 5, 3, CWIDTH, height - 6, 0);
                                tft.fillRect(tft.textWidth(content.substring(0, cursorPos + 1)) + c_offset + 5, 3, CWIDTH, height - 6, 0xffff);
                            }
                            break;
                        case RIGHT:
                            if (cursorPos < content.length()) {
                                cursorPos++;
                                length = tft.textWidth(content.substring(0, cursorPos)) + 15;
                                if (length > width)
                                    c_offset = width - length;
                                tft.fillRect(xpos, dypos, width, height, 0xFFFF);
                                tft.drawRect(xpos, dypos, width, height, 0xF800);
                                tft.setCursor(xpos + 5 + c_offset, dypos + yoff);
                                tft.println(content);
                                tft.fillRect(tft.textWidth(content.substring(0, cursorPos - 1)) + c_offset + 5, 3, CWIDTH, height - 6, 0xffff);
                                tft.fillRect(tft.textWidth(content.substring(0, cursorPos)) + c_offset + 5, 3, CWIDTH, height - 6, 0);
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
                        default:
                            *direction = BACK;
                            break;
                        }
                        c = buttonsHelding();
                    }
                }
            }
        }

    tft.fillRect(tft.textWidth(content.substring(0, cursorPos)) + c_offset + 5, 3, CWIDTH, height - 6, 0xFFFF);
    tft.fillRect(xpos, dypos, width, height, 0xFFFF);
    tft.drawRect(xpos, dypos, width, height, 0);
    tft.setCursor(xpos + 5, dypos + yoff);
    tft.println(content);
    tft.resetViewport();
    tft.setCursor(5, ypos - 5);
    changeFont(1);
    tft.setTextSize(1);
    tft.setTextColor(0);
    tft.print(title);
    content.trim();
    return content;
}
// Animation of spinning circles in rectangle, used in outgoing call
void spinAnim(int x, int y, int size_x, int size_y, int offset, int spacing) {
    // FIRSTLY WAS WRITED MANUALLY BUT AFTER ENCOURING A BUG
    // I WROTE THIS WITH HELP FROM CHATGPT
    //

    //  Open the file and seek to the position where image data starts
    File file = SD.open(resPath);
    file.seek(0x658BC4);
    // Read data from SD card into buffer
    const int buffer_size = 400 * 2;
    uint8_t   buffer[buffer_size];
    file.read(buffer, buffer_size);
    file.close();

    // Define max frames and set up variables for animation path
    int  max_count     = (2 * size_x) + (2 * (size_y - 1)); // Adjusted max_count for full frames
    int  printed_count = 0;
    int  xt            = 0;
    int  yt            = 0;
    bool draw          = true;

    // Array for the current 7x7 frame, as uint16_t (16-bit color for pushImage)
    uint16_t currentcircle[49]; // 7x7 frame requires 49 pixels

    // Main animation loop
    while (printed_count < max_count) {
        for (int j = offset; j >= 0 && printed_count < max_count; j--) {
            if (draw) {
                int start_index = 98 * j; // Starting index for the 7x7 frame in buffer

                // Ensure we're within buffer bounds
                if (start_index + 98 <= buffer_size) {
                    for (int i = 0; i < 49; i++) {
                        // Convert two bytes per pixel into 16-bit color
                        currentcircle[i] = (buffer[start_index + (i * 2)] << 8) | buffer[start_index + (i * 2) + 1];
                    }

                    // Display the frame at the calculated position
                    tft.pushImage(x + xt, y + yt, 7, 7, currentcircle);
                }
            }

            if (printed_count < size_x) {
                xt += spacing;
            } else if (printed_count < size_x + size_y - 1) {
                yt += spacing;
            } else if (printed_count < (2 * size_x) + size_y - 1) {
                xt -= spacing;
            } else { // Move up
                yt -= spacing;
                if (yt <= -((size_y - 1) * spacing)) {
                    draw = false; // End animation once reaching the top again
                }
            }

            printed_count++;
        }
        offset = 7; // Reset offset after each loop iteration
    }
}

int lastpercentage;
// progress bar that used on boot screen
void progressBar(int val, int max, int y, int h, uint16_t color, bool log, bool fast) {

    int percentage = (val * 100) / max;
    if (lastpercentage > percentage)
        lastpercentage = percentage;
    if (!log) {
        //"for loop" and delay for smooth transition
        tft.drawRect(69, y, 100, h, color);
        for (int i = lastpercentage; i <= percentage; i++) {
            tft.fillRect(69, y, i, h, color);
            if (!fast)
                delay(5);
        }
    } else {
#ifndef LOG
        tft.drawRect(69, y, 100, h, color);
        for (int i = lastpercentage; i <= percentage; i++) {
            tft.fillRect(69, y, i, h, color);
            if (!fast)
                delay(5);
        }
#endif
    }
    lastpercentage = percentage;
}
// ## Critical system error
//  This function is called when a critical error occurs in the system.
void sysError(String reason) {
    tft.fillScreen(0x0000);
    tft.setCursor(10, 40);
    tft.setTextFont(1);
    tft.setTextSize(4);
    tft.setTextColor(0xF001); // FOOL :3
    tft.println("==ERROR==");
    tft.setTextColor(0xFFFF);
    tft.setTextSize(1);
    tft.println(String("\n\n\nThere a problem with your device\n\nTechnical details:\n\n\nReason:" + reason + "\n\n\nyou can found contact info at\n\n" REPOSITORY_LINK "\n\n"));

    tft.println("Press any button to restart\nor reset button to reset the device");

    while (buttonsHelding(false) == -1)
        ;
    ESP.restart();
    for (;;)
        ;
}
