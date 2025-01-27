#include "render.h"
const int lastImage = 42;
bool      button(String title, int xpos, int ypos, int w, int h, bool selected, int *direction) {

    tft.fillRect(xpos, ypos, w, h, 0xFFFF);
    tft.drawRect(xpos, ypos, w, h, 0);

    if (selected) {
        tft.setTextColor(0xF800);
        tft.drawRect(xpos, ypos, w, h, 0);
        tft.drawRect(xpos, ypos, w, h, 0xF800);
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
            char c = buttonsHelding();
            switch (c) {
            case SELECT:
                return true;
                break;
            case LEFT:
                *direction = LEFT;
                exit       = true;
                break;
            case RIGHT:
                *direction = RIGHT;
                exit       = true;
                break;
            case UP:
                *direction = UP;
                exit       = true;
                break;
            case DOWN:
                *direction = DOWN;
                exit       = true;
                break;
            }
        }
    tft.setTextColor(0);
    tft.fillRect(xpos, ypos, w, h, 0xFFFF);
    tft.drawRect(xpos, ypos, w, h, 0);
    tft.setCursor(xpos + x, ypos + y);
    tft.print(title);
    return false;
}
String textbox(String title, String content, int ypos, bool onlydraw, bool selected, bool used, int *direction, bool onlynumbers) {

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
    int charWidth = tft.textWidth("D") + 1;
    int xpos      = 5;
    int width     = 235 - xpos;
    int height    = 25;

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
                            if (TI != '\n')
                                if (TI != '\b') {
                                    content = content.substring(0, cursorPos) + TI + content.substring(cursorPos, content.length());
                                    cursorPos++;
                                } else {
                                    content = content.substring(0, cursorPos - 1) + content.substring(cursorPos, content.length());
                                    cursorPos--;
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

void changeFont(int ch) {
    currentFont = ch;
    switch (ch) {
    case 0:
        tft.setTextFont(1);
        break;
    case 1:
        tft.setFreeFont(&FreeSans9pt7b);
        break;
    case 2:
        tft.setFreeFont(&FreeSansBold9pt7b);
        break;
    case 3:
        tft.setFreeFont(&FreeMono9pt7b);
        break;
    case 4:
        tft.setFreeFont(&FreeSans12pt7b);
        break;
    }
}
void *pngOpen(const char *filename, int32_t *size) {
    File *file = new File(SD.open(filename, FILE_READ));
    *size      = file->size();
    return file;
}

void pngClose(void *handle) {
    File *file = (File *)handle;
    if (file) {
        file->close();
        delete file;
    }
}

int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length) {
    File *file = (File *)page->fHandle;
    return file->read(buffer, length);
}

int32_t pngSeek(PNGFILE *page, int32_t position) {
    File *file = (File *)page->fHandle;
    return file->seek(position);
}

void pngDraw(PNGDRAW *pDraw) {
    uint16_t lineBuffer[240];
    png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
    tft.pushImage(0, pDraw->y + 26, pDraw->iWidth, 1, lineBuffer);
}

// Function to draw the PNG image
void drawPNG(const char *filename) {
    File file = SD.open(filename, FILE_READ);
    if (!file) {
        Serial.println("Failed to open file");
        return;
    }

    int rc = png.open(filename, pngOpen, pngClose, pngRead, pngSeek, pngDraw);
    if (rc == PNG_SUCCESS) {
        Serial.printf("Image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
        rc = png.decode(NULL, 0);
        png.close();
    } else {
        Serial.printf("PNG open failed: %d\n", rc);
    }

    file.close();
}

// Draw and downscale image
void drawFromSdDownscale(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, int scale, File file) {
    // PARTIALLY STOLEN FROM CHATGPT
    if (!file.available())
        sysError("SD_CARD_NOT_AVAILABLE");
    file.seek(pos);

    const int downscaled_width = size_x / scale;
    const int buffer_size      = size_x * 2; // 2 bytes per pixel
    uint8_t   buffer[buffer_size];

    for (int a = 0; a < size_y; a += scale) {
        file.read(buffer, buffer_size);

        uint16_t line[downscaled_width];
        for (int i = 0; i < downscaled_width; i++) {
            line[i] = (buffer[2 * i * scale] << 8) | buffer[2 * i * scale + 1];
        }

        tft.pushImage(pos_x, pos_y + (a / scale), downscaled_width, 1, line);

        // Skip the lines that won't be drawn (vertical downscaling)
        file.seek(file.position() + (size_x * 2 * (scale - 1)));
    }
    file.close();
}
int8_t _signal = 0;
int8_t charge  = 0;
void   drawStatusBar() {
    bool messageViewport = tft.getViewportY() == 51;
    if (sBarChanged) {
        if (messageViewport) {
            tft.resetViewport();
        }
        sBarChanged = false;
        charge      = getChargeLevel();
        drawFromSd(0X5A708D, 0, 0, 240, 26); // statusbar
        if (_signal == -1)
            drawFromSd(0x5AD47D, 0, 0, 37, 26); // no_signal
        else
            drawFromSd(0x5ABC1D + (0x618) * _signal, 0, 0, 30, 26); // signal
        drawFromSd(0X5AA14D + (0x6B4) * charge, 207, 0, 33, 26);    // battery
        //  tft.print(String(charge) + String("%"));
        if (messageViewport) {
            tft.setViewport(0, 51, 240, 269, true);
        }
    }
}

void drawCutoutFromSd(SDImage image,
                      int cutout_x, int cutout_y,
                      int cutout_width, int cutout_height,
                      int display_x, int display_y,
                      File file) {

    if (!file || !file.available()) {
        Serial.println("Failed to open file or file not available");
        return;
    }

    int image_width = image.w;

    uint32_t start_offset = image.address + (cutout_y * image_width + cutout_x) * 2;

    const int buffer_size = cutout_width * 2;
    uint8_t   buffer[buffer_size];

    for (int row = 0; row < cutout_height; row++) {

        uint32_t row_offset = start_offset + (row * image_width * 2);
        file.seek(row_offset);

        int bytesRead = file.read(buffer, buffer_size);
        if (bytesRead != buffer_size) {
            Serial.println("Error reading row from SD card.");
            return;
        }
        tft.pushImage(display_x, display_y + row, cutout_width, 1, (uint16_t *)buffer);
    }
}

void rendermenu(int choice, bool right) {
    const uint32_t baseAddress = 0x5D5097;
    const uint32_t iconOffset  = 0x17A2;

    const SDImage onIcons[] = {
        {0x5D0341, 49, 49, 0, false}, // First on icon
        {0x5D1603, 49, 51, 0, false}, // Second on icon
        {0x5D2989, 50, 50, 0, false}, // Third on icon
        {0x5D3D11, 49, 51, 0, false}  // Fourth on icon
    };

    const struct {
        int x, y;
    } IconPositions[] = {
        {49, 43 + 26}, // On Icons
        {138, 42 + 26},
        {49, 122 + 26},
        {137, 123 + 26},
        {51, (45 + 26)}, // Off Icons
        {141, (44 + 26)},
        {52, (125 + 26)},
        {140, (125 + 26)}};

    int offIndex;
    if (right) {
        offIndex = (choice == 0) ? 3 : choice - 1;
    } else {
        offIndex = (choice + 1) % 4;
    }

    int onIndex = choice;

    drawFromSd(
        IconPositions[offIndex].x,
        IconPositions[offIndex].y,
        SDImage(baseAddress + (iconOffset * offIndex), 55, 55, 0, false));

    drawFromSd(
        IconPositions[onIndex + 4].x,
        IconPositions[onIndex + 4].y,
        onIcons[onIndex]);
}

void sysError(const char *reason) {
    tft.fillScreen(0x0000);
    tft.setCursor(10, 40);
    tft.setTextFont(1);
    tft.setTextSize(4);
    tft.setTextColor(0xF001); // FOOL :3
    tft.println("==ERROR==");
    tft.setTextColor(0xFFFF);
    tft.setTextSize(1);
    tft.println(String("\n\n\nThere a problem with your device\nYou can fix it by yourself i guess\nThere some details for you:\n\n\nReason:" + String(reason)));
    for (;;)
        ;
}

void drawFromSd(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, File file, bool transp, uint16_t tc) {
    if (!file.available())
        sysError("SD_CARD_NOT_AVAILABLE");
    file.seek(pos);
    if (!transp) {
        const int buffer_size = size_x * 2;
        uint8_t   buffer[buffer_size];

        for (int a = 0; a < size_y; a++) {
            file.read(buffer, buffer_size);

            tft.pushImage(pos_x, a + pos_y, size_x, 1, (uint16_t *)buffer);
        }
    } else {
        const int buffer_size = size_x * 2; // 2 bytes per pixel
        uint8_t   buffer[buffer_size];

        for (int a = 0; a < size_y; a++) {
            // Read a whole line (row) of pixels at once
            file.read(buffer, buffer_size);

            int draw_start = -1; // Initialize start of draw segment

            for (int i = 0; i < size_x; i++) {
                // Reconstruct 16-bit color from two bytes
                uint16_t wd = (buffer[2 * i] << 8) | buffer[2 * i + 1];

                if (wd != tc) { // If the pixel is not transparent
                    if (draw_start == -1) {
                        draw_start = i; // Start new draw segment
                    }
                } else { // Transparent pixel
                    if (draw_start != -1) {
                        // Render segment up to current pixel
                        tft.pushImage(pos_x + draw_start, pos_y + a, i - draw_start, 1, (uint16_t *)(&buffer[2 * draw_start]));
                        draw_start = -1; // Reset draw_start
                    }
                }
            }

            // Handle case where last segment reaches the end of the row
            if (draw_start != -1) {
                tft.pushImage(pos_x + draw_start, pos_y + a, size_x - draw_start, 1, (uint16_t *)(&buffer[2 * draw_start]));
            }
        }
    }
}

void drawFromSd(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, bool transp, uint16_t tc) {
    drawFromSd(pos, pos_x, pos_y, size_x, size_y, SD.open("/FIRMWARE/IMAGES.SG"), transp, tc);
}

void drawFromSd(int x, int y, SDImage sprite) {
    drawFromSd(sprite.address, x, y, sprite.w, sprite.h, sprite.transp, sprite.tc);
}
void writeCustomFont(int x, int y, String input, int type) {
    int      w      = 21;
    int      h      = 27;
    bool     transp = false;
    uint16_t tc     = 0xFFFF;
    uint32_t addr   = 0x658ED3;
    switch (type) {
    case 0:
        break;
    case 1:
        w      = 15;
        h      = 19;
        addr   = 0x65FBC7;
        transp = true;
        break;
    default:
        break;
    }
    int size = w * h * 2;

    char buf[64];
    input.toCharArray(buf, 64);
    for (int i = 1; i < input.length() + 1; i++) {
        Serial.println(String(buf[i - 1]) + " " + String(w * (i - 1) + x));
        switch (buf[i - 1]) {
        case '#':
            drawFromSd(addr + (size * 10), w * (i - 1) + x, y, w, h, transp, tc);
            break;
        case '*':
            drawFromSd(addr + (size * 11), w * (i - 1) + x, y, w, h, transp, tc);
            break;
        case 'X':
            drawFromSd(addr + (size * 12), w * (i - 1) + x, y, w, h, transp, tc);
            break;
        case '/':
            drawFromSd(addr + (size * 13), w * (i - 1) + x, y, w, h, transp, tc);
            break;
        case ':':
            drawFromSd(addr + (size * 14), w * (i - 1) + x, y, w, h, transp, tc);
            break;
        default:
            if (buf[i - 1] > 47 && buf[i - 1] < 58)
                drawFromSd(addr + (size * (buf[i - 1] - 48)), w * (i - 1) + x, y, w, h, transp, tc);
            break;
        }
    }
}

void listMenu_sub(String label, int type, int page, int pages) {

    tft.setCursor(30, 45);
    tft.setTextSize(1);
    changeFont(1);
    tft.setTextColor(0xffff);
    tft.print(label);
    changeFont(0);
    tft.setCursor(210, 40);
    tft.print(String(page + 1) + String("/") + String(pages + 1));
    changeFont(1);
}

void lM_entryRender(int x, int y, int i, int index, int scale, int height, int icon_x, mOption choice, bool images = false) {
    if (images) {

        tft.drawLine(0, 50 + height * (i + 1), 240, 50 + height * (i + 1), 0x0000);
        tft.drawLine(0, 50 + height * i, 240, 50 + height * i, 0x0000);
        if (index != lastImage) // Do not draw image if its "Pick wallpaper"
            drawFromSdDownscale((uint32_t)(0xE) + ((uint32_t)(0x22740) * index), 10, 51 + height * i, 240, 294, scale);
    } else if (choice.icon.address != 0) {
        drawFromSd(icon_x, 51 + height * i, choice.icon);
    }
    tft.setCursor(x, y + (height * i));
    tft.print(choice.label);
}

int listMenu(mOption *choices, int icount, bool images, int type, String label, bool forceIcons, int findex) {
    tft.setTextWrap(false, false);
    /*

    int icount
    I know I just can count here with ArraySize but whatever
    Length counts wrong with causes OutOfBounds panic thing
    This is Application for ESP32 writed within arduino environment

    label = Title of Menu

    bool images = Is it images? (Is it wallpaper choice will be more correct)

    int type
    0 = MESSAGES
    1 = CONTACTS
    2 = SETTINGS


        To be honest, this was one of the first functions that I wrote here
        Now, I looking at this and my mind is not working properly
        I feel like I can delete half of those lines...
        And it will work properly...
        Not mind, code.

        At some point I will learn how to write clean code...
    */
    bool empty = false;
    if (icount == 0)
        empty = true;

    // load file with graphical resources
    int scale  = 7;  // downscale multiplier for images
    int x      = 10; // coordinates where begin to render text
    int y      = 65;
    int mult   = 20; // height of entry
    int icon_x = 2;

    int indexx = 0;
    if (images) {
        mult = (294 / scale) + 1;
        x    = 55;
    } else if (choices[0].icon.address != 0 || forceIcons) {
        x = 29;
    }
    tft.fillRect(0, 26, 240, 25, 0x32B3);
    // drawFromSd(0x5DAF1F, 0, 26, 240, 25); // bluebar TODO: just draw rectangle instead of loading a whole image
    if (type >= 0 && type < 3) // with icon to draw...
        drawFromSd(0x5DDDFF + (0x4E2 * type), 0, 26, 25, 25);
    int items_per_page = 269 / mult; // max items per page (empty space height divided by spacing)
    int count          = icount;
    int pages          = (icount + items_per_page - 1) / items_per_page - 1;
    int page           = pages > 0 ? findex / pages : 0;
    int icon_addr;

    uint16_t color_active   = 0xFDD3;
    uint16_t color_inactive = 0x0000;
    int      choice         = findex;

    tft.setTextSize(1);
    tft.setTextColor(color_inactive);
    changeFont(1);

    listMenu_sub(label, type, page, pages);
    drawFromSd(0x639365, 0, 51, 240, 269);
    tft.setTextColor(color_inactive);
    if (empty) {
        tft.setCursor(75, 70);
        tft.print("< Empty >");
        while (buttonsHelding() == -1)
            idle();
        return -2;
    }
    tft.fillRect(0, 51 + mult * choice, 240, mult, color_active);
    for (int i = 0; i < items_per_page && items_per_page * page + i < icount; i++) {
        indexx = items_per_page * page + i;
        lM_entryRender(x, y, i, indexx, scale, mult, icon_x, choices[indexx], images);
    }
    bool exit = false;
    while (!exit) {
        switch (buttonsHelding()) {
        case SELECT: {
            exit = true;

            return items_per_page * page + choice;
            break;
        }
        case UP: { // Up button

            bool changed    = false;
            int  old_choice = choice;
            if (choice > 0) {
                choice--;
                tft.fillRect(0, 51 + mult * choice, 240, mult, color_active);
            } else if (page > 0) {

                drawFromSd(0x5DAF1F, 0, 26, 240, 25);
                if (type >= 0 && type < 3)
                    drawFromSd(0x5DDDFF + (0x4E2 * type), 0, 26, 25, 25);
                page--;
                changed = true;
                drawFromSd(0x639365, 0, 51, 240, 269);
                choice = (items_per_page - 1);
                tft.fillRect(0, 51 + mult * choice, 240, mult, color_active);
                for (int i = 0; i < items_per_page && items_per_page * page + i < icount; i++) {
                    indexx = items_per_page * page + i;
                    lM_entryRender(x, y, i, indexx, scale, mult, icon_x, choices[indexx], images);
                }

            } else {

                drawFromSd(0x5DAF1F, 0, 26, 240, 25);

                if (type >= 0 && type < 3)
                    drawFromSd(0x5DDDFF + (0x4E2 * type), 0, 26, 25, 25);

                page    = pages;
                changed = true;

                drawFromSd(0x639365, 0, 51, 240, 269);
                choice = (icount % items_per_page) ? (icount % items_per_page) - 1 : (items_per_page - 1);
                tft.fillRect(0, 51 + mult * choice, 240, mult, color_active);
                for (int i = 0; i < items_per_page && items_per_page * page + i < icount; i++) {
                    indexx = items_per_page * page + i;
                    lM_entryRender(x, y, i, indexx, scale, mult, icon_x, choices[indexx], images);
                }
            }

            if (!changed)
                drawFromSd(0x639365 + (mult * old_choice * 240 * 2), 0, 51 + (mult * old_choice), 240, mult);

            listMenu_sub(label, type, page, pages);
            tft.setTextColor(color_inactive);
            tft.setCursor(x, y + (mult * old_choice));
            if (!changed) {
                indexx = items_per_page * page + old_choice;
                lM_entryRender(x, y, old_choice, indexx, scale, mult, icon_x, choices[indexx], images);
                ;
                tft.setCursor(x, y + (mult * choice));
                tft.print(choices[items_per_page * page + choice].label);
                indexx = items_per_page * page + choice;
                lM_entryRender(x, y, choice, indexx, scale, mult, icon_x, choices[indexx], images);
            }

            break;
        }
        case DOWN: { // Down button
            int  old_choice = choice;
            bool changed    = false;
            if (choice < (items_per_page - 1) && items_per_page * page + choice < icount - 1) {
                choice++;
            } else if (page < pages) {
                page++;
                changed = true;
                choice  = 0;
                drawFromSd(0x639365, 0, 51, 240, 269);
                tft.fillRect(0, 51 + mult * choice, 240, mult, color_active);
                for (int i = 0; i < items_per_page && items_per_page * page + i < icount; i++) {
                    indexx = items_per_page * page + i;
                    lM_entryRender(x, y, i, indexx, scale, mult, icon_x, choices[indexx], images);
                }
                drawFromSd(0x5DAF1F, 0, 26, 240, 25);
                if (type >= 0 && type < 3)
                    drawFromSd(0x5DDDFF + (0x4E2 * type), 0, 26, 25, 25);
            } else {
                page    = 0;
                changed = true;
                choice  = 0;
                drawFromSd(0x639365, 0, 51, 240, 269);
                tft.fillRect(0, 51 + mult * choice, 240, mult, color_active);
                for (int i = 0; i < items_per_page && items_per_page * page + i < icount; i++) {
                    indexx = items_per_page * page + i;
                    lM_entryRender(x, y, i, indexx, scale, mult, icon_x, choices[indexx], images);
                }

                drawFromSd(0x5DAF1F, 0, 26, 240, 25);
                if (type >= 0 && type < 3)
                    drawFromSd(0x5DDDFF + (0x4E2 * type), 0, 26, 25, 25);
            }
            if (!changed) {

                drawFromSd(0x639365 + (mult * old_choice * 240 * 2), 0, 51 + (mult * old_choice), 240, mult);
            }
            listMenu_sub(label, type, page, pages);
            tft.setTextColor(color_inactive);
            tft.setCursor(x, y + (mult * old_choice));
            if (!changed) {

                indexx = items_per_page * page + old_choice;
                lM_entryRender(x, y, old_choice, indexx, scale, mult, icon_x, choices[indexx], images);

                tft.setCursor(x, y + (mult * choice));
                tft.fillRect(0, 51 + mult * choice, 240, mult, color_active);
                tft.print(choices[items_per_page * page + choice].label);

                indexx++;
                lM_entryRender(x, y, choice, indexx, scale, mult, icon_x, choices[indexx], images);
            }

            break;
        }
        case BACK: { // EXIT
            exit = true;
            return -1;
            break;
        }
        }
        idle();
    }

    return -1;
}

int listMenu(const String choices[], int icount, bool images, int type, String label, bool forceIcons, int findex) {
    mOption *optionArr = new mOption[icount];
    for (int i = 0; i < icount; i++) {
        optionArr[i].label = choices[i];
        optionArr[i].icon  = SDImage();
    }
    return listMenu(optionArr, icount, images, type, label, forceIcons, findex);
}

void spinAnim(int x, int y, int size_x, int size_y, int offset, int spacing) {
    // FIRSTLY WAS WRITED MANUALLY BUT AFTER ENCOURING A BUG
    // I STOLE FROM CHATGPT
    //(slighty modified)
    //  Open the file and seek to the position where image data starts
    File file = SD.open("/FIRMWARE/IMAGES.SG");
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

int choiceMenu(const String choices[], int count, bool context) {
    // TODO ?
    int      x              = 30;
    int      y              = 95;
    int      mul            = 20;
    uint16_t color_active   = 0xF9C0;
    uint16_t color_inactive = 0x0000;

    if (context) {
        drawFromSd(0x607565, 16, 100, 208, 123, true, 0x07e0);
        x = 40;
        y = 120;
    } else {
        drawFromSd(0x5E8A25, 0, 68, 240, 128);
        x = 30;
        y = 95;
    }

    // tft.setCursor(36, 95);
    tft.setTextSize(1);
    tft.setTextColor(color_inactive);
    changeFont(1);

    for (int i = 0; i < count; i++) {
        tft.setCursor(x, y + (mul * i));
        tft.print(choices[i]);
    }

    int choice = 0;
    tft.setTextColor(color_active);
    tft.setCursor(x, y + (mul * choice));
    tft.print(choices[choice]);
    int yy = (y + (mul * choice)) - 10;
    int xx = x - 5;
    tft.fillTriangle(xx - 10, yy, xx - 10, yy + 10, xx - 2, yy + 5, color_active);
    bool exit = false;
    while (!exit) {
        switch (buttonsHelding()) {
        case BACK: {
            exit = true;

            return -1;
            break;
        }
        case UP: {
            // left
            if (context) {
                tft.setTextSize(1);
                tft.setTextColor(color_inactive);

                drawFromSd(0x607565, 16, 100, 208, 123, true, 0x07e0);
                for (int i = 0; i < count; i++) {
                    tft.setCursor(x, y + (mul * i));
                    tft.print(choices[i]);
                }
            } else {
                tft.setTextSize(1);
                tft.setTextColor(color_inactive);

                drawFromSd(0x5E8A25, 0, 68, 240, 128);
                for (int i = 0; i < count; i++) {
                    tft.setCursor(x, y + (mul * i));
                    tft.print(choices[i]);
                }
            }
            if (choice < 1) {
                choice = count - 1;
            } else {
                choice--;
            }
            tft.setTextColor(color_inactive);
            tft.setCursor(x, y + (mul * (choice + 1)));
            if (choice < count - 1)
                tft.print(choices[choice + 1]);
            if (choice == count - 1) {
                tft.setCursor(x, y);
                tft.print(choices[0]);
            }
            tft.setTextColor(color_active);
            tft.setCursor(x, y + (mul * choice));
            int yy = (y + (mul * choice)) - 10;
            int xx = x - 5;
            tft.fillTriangle(xx - 10, yy, xx - 10, yy + 10, xx - 2, yy + 5, color_active);
            tft.print(choices[choice]);
            delay(100);
            break;
        }
        case SELECT: {
            // middle

            exit = true;
            return choice;
            break;
        }
        case DOWN: {
            // right
            if (context) {
                tft.setTextSize(1);
                tft.setTextColor(color_inactive);

                drawFromSd(0x607565, 16, 100, 208, 123, true, 0x07e0);
                for (int i = 0; i < count; i++) {
                    tft.setCursor(x, y + (mul * i));
                    tft.print(choices[i]);
                }
            } else {
                tft.setTextSize(1);
                tft.setTextColor(color_inactive);

                drawFromSd(0x5E8A25, 0, 68, 240, 128);
                for (int i = 0; i < count; i++) {
                    tft.setCursor(x, y + (mul * i));
                    tft.print(choices[i]);
                }
            }
            if (choice > count - 2) {
                choice = 0;
            } else {
                choice++;
            }

            tft.setTextColor(color_inactive);
            tft.setCursor(x, y + (mul * (choice - 1)));

            if (choice > 0)
                tft.print(choices[choice - 1]);

            if (choice == 0) {
                tft.setCursor(x, y + (mul * (count - 1)));
                tft.print(choices[count - 1]);
            }
            tft.setTextColor(color_active);
            tft.setCursor(x, y + (mul * choice));
            int yy = (y + (mul * choice)) - 10;
            int xx = x - 5;
            tft.fillTriangle(xx - 10, yy, xx - 10, yy + 10, xx - 2, yy + 5, color_active);
            tft.print(choices[choice]);

            delay(100);
            break;
        }
        }
        idle();
    }
    return -1;
}
// void findSplitPosition(String text, int &charIndex, int &posX, int &posY, int direction)
// {

//   int prevNL = 0;
//   int lastNewLine = 0;
//   int curPosInText = 0;
//   for (; curPosInText < charIndex; curPosInText++)
//   {
//     posX = tft.textWidth(text.substring(lastNewLine, curPosInText));
//     if (posX >= 240 || text[curPosInText] == '\n')
//     {
//       posX = 0;
//       prevNL = lastNewLine;
//       lastNewLine = --curPosInText;
//       posY += tft.fontHeight();
//     }
//     if (curPosInText >= text.length() - 1)
//     {
//       curPosInText = text.length() - 1;
//       break;
//     }
//   }
//   int lastX = posX;
//   if (direction == UP)
//   {
//     if (lastNewLine != 0)
//     {
//       for (int i = lastNewLine; posX > lastX; i--)
//       {
//         posX = tft.textWidth(text.substring(prevNL, i));
//         if (curPosInText >= text.length() - 1)
//         {
//           curPosInText = text.length() - 1;
//           break;
//         }
//       }
//       posY -= tft.fontHeight();
//     }
//   }
//   else if (direction == DOWN)
//   {
//     while (text[curPosInText] != '\n' || posX < 240)
//     {
//       if (curPosInText < text.length())
//         curPosInText++;
//       posX = tft.textWidth(text.substring(lastNewLine, curPosInText));
//       if (curPosInText >= text.length() - 1)
//       {
//         curPosInText = text.length() - 1;
//         break;
//       }
//     }

//     posY += tft.fontHeight();
//     while (posX < lastX)
//     {
//       curPosInText++;
//       if (curPosInText >= text.length() - 1)
//       {
//         curPosInText = text.length() - 1;
//         break;
//       }
//     }
//   }
//   if (charIndex >= text.length() - 1)
//     charIndex = text.length() - 1;
//   if (charIndex < 0)
//     charIndex = 0;
// }
void findSplitPosition(String text, int charIndex, int &posX, int &posY, int direction) {
    int prevNL       = 0; // Last newline position
    int lastNewLine  = 0; // Current line's start position
    int curPosInText = 0; // Current character position
    posX             = 0; // Reset position to 0 at the start
    posY             = 0; // Reset vertical position

    // Loop through text up to the charIndex
    for (; curPosInText < charIndex; curPosInText++) {
        posX = tft.textWidth(text.substring(lastNewLine, curPosInText + 1));

        // If the position exceeds 240px width or we encounter a newline
        if (posX >= 240 || text[curPosInText] == '\n') {
            posX        = 0;                // Reset X position for the next line
            prevNL      = lastNewLine;      // Remember the start of the previous line
            lastNewLine = curPosInText + 1; // Set new line's start
            posY += tft.fontHeight();       // Move to the next line vertically
        }

        if (curPosInText >= text.length() - 1) {
            break; // Stop if we're at the end of the text
        }
    }
}
int findCharIndex(String text, int &charIndex, int direction) {
    int prevNL       = 0;             // Start of the previous line
    int lastNewLine  = 0;             // Start of the current line
    int nextNL       = text.length(); // End of the current line
    int curPosInText = 0;             // Current character position
    int posX         = 0;             // Current X position
    int targetX      = 0;             // Desired X position

    // Find the current line and character's position in that line
    for (; curPosInText <= charIndex && curPosInText < text.length(); curPosInText++) {
        posX = tft.textWidth(text.substring(lastNewLine, curPosInText));

        if (posX >= 240 || text[curPosInText] == '\n') {
            prevNL      = lastNewLine;
            lastNewLine = curPosInText + 1;
            posX        = 0;
        }

        if (curPosInText == charIndex) {
            targetX = posX; // Store the X position of the current character
        }
    }

    // Find the end of the current line
    for (int i = lastNewLine; i < text.length(); i++) {
        if (text[i] == '\n' || tft.textWidth(text.substring(lastNewLine, i + 1)) >= 240) {
            Serial.println("=========================");
            Serial.println(i);
            nextNL = i;
            break;
        }
    }

    // Handle UP direction
    if (direction == UP && prevNL >= 0) {
        // Find the character at targetX in the previous line
        curPosInText = prevNL;
        for (; curPosInText < lastNewLine; curPosInText++) {
            posX = tft.textWidth(text.substring(prevNL, curPosInText + 1));
            if (posX >= targetX + 5 || text[curPosInText] == '\n') {
                break;
            }
        }
    }
    // Handle DOWN direction
    else if (direction == DOWN && nextNL < text.length()) {
        // Move to the start of the next line
        lastNewLine = nextNL + 1;
        Serial.println("=====DOWN======");
        Serial.println(String(lastNewLine) + "=====" + String(text.length()));
        // Find the end of the next line
        for (int i = lastNewLine; i < text.length(); i++) {
            if (text[i] == '\n' || tft.textWidth(text.substring(lastNewLine, i + 1)) >= 240 || i == text.length() - 1) {
                nextNL = i;
                break;
            }
        }
        Serial.println("============");
        Serial.println(targetX);
        // Find the character at targetX in the next line
        curPosInText = lastNewLine;
        posX         = 0;
        for (; targetX >= posX; curPosInText++) {
            posX = tft.textWidth(text.substring(lastNewLine, curPosInText + 1));
            if (posX >= targetX || text[curPosInText] == '\n' || curPosInText == text.length()) {
                break;
            }
        }
    }

    // Ensure charIndex is within valid bounds
    charIndex = constrain(curPosInText, 0, text.length() - 1);
    return charIndex;
}

String SplitString(String text) {
    String result           = "";            // Final string with newlines
    int    wordStart        = 0;             // Start index of a word
    int    wordEnd          = 0;             // End index of a word
    int    textLen          = text.length(); // Total length of the input text
    int    currentLineWidth = 0;             // Width of the current line

    while (wordStart < textLen) {
        // Check for existing newline
        int newlinePos = text.indexOf('\n', wordStart);
        if (newlinePos != -1 && (newlinePos < text.indexOf(' ', wordStart) || text.indexOf(' ', wordStart) == -1)) {
            // Add the text up to the newline to the result
            result += text.substring(wordStart, newlinePos + 1);
            wordStart        = newlinePos + 1; // Move past the newline
            currentLineWidth = 0;              // Reset line width
            continue;
        }

        // Find the next space or the end of the text
        wordEnd = text.indexOf(' ', wordStart);
        if (wordEnd == -1)
            wordEnd = textLen; // If no more spaces, set to the end of the text

        String   word    = text.substring(wordStart, wordEnd);
        uint16_t wordLen = tft.textWidth(word);

        // Check if the word fits in the current line
        if (currentLineWidth + wordLen > tft.width() && currentLineWidth > 0) {
            result += "\n";       // Add a newline to the result
            currentLineWidth = 0; // Reset line width
        }

        // Add the word to the result
        result += word;

        // Add a space if there are more words and we're not at the end of the text
        if (wordEnd < textLen) {
            result += " ";
            currentLineWidth += tft.textWidth(" "); // Account for space width
        }

        currentLineWidth += wordLen; // Update the line width
        wordStart = wordEnd + 1;     // Move to the next word
    }

    return result; // Return the final string with newlines
}

void drawWallpaper() {
    if (SD.exists(currentWallpaperPath))
        drawPNG(currentWallpaperPath.c_str());
    else if  (wallpaperIndex < 0|| wallpaperIndex>42)
        drawFromSd((uint32_t)(0xD) + ((uint32_t)(0x22740) * wallpaperIndex), 0, 26, 240, 294);
    else {
        wallpaperIndex = 0;
        drawFromSd((uint32_t)(0xD) + ((uint32_t)(0x22740) * wallpaperIndex), 0, 26, 240, 294);
    }
}