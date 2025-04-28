#include "render.h"

const int lastImage = 42;
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

void chfont(const GFXfont *f, bool is_screen_buffer, TFT_eSprite &sbuffer) {
    if (is_screen_buffer)
        sbuffer.setFreeFont(f);
    else
        tft.setFreeFont(f);
}
void chfont(uint8_t f, bool is_screen_buffer, TFT_eSprite &sbuffer) {
    if (is_screen_buffer)
        sbuffer.setTextFont(f);
    else
        tft.setTextFont(f);
}
void changeFont(int ch, bool is_screen_buffer, TFT_eSprite &sbuffer) {
    currentFont = ch;
    switch (ch) {
    case 0:
        chfont(1, is_screen_buffer, sbuffer);
        break;
    case 1:
        chfont(&FreeSans9pt7b, is_screen_buffer, sbuffer);
        break;
    case 2:
        chfont(&FreeSansBold9pt7b, is_screen_buffer, sbuffer);
        break;
    case 3:
        chfont(&FreeMono9pt7b, is_screen_buffer, sbuffer);
        break;
    case 4:
        chfont(&FreeSans12pt7b, is_screen_buffer, sbuffer);
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

bool iswallpaper = false;
void pngDraw(PNGDRAW *pDraw) {
    uint16_t lineBuffer[240];
    png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);

    if (iswallpaper) {
        if (pDraw->y < 294) {
            memcpy(wallpaper + (pDraw->y * 480), (uint8_t *)lineBuffer, 480);
        }
    } else {
        tft.pushImage(0, pDraw->y + 26, pDraw->iWidth, 1, lineBuffer);
    }
}

// ## Draw PNG image
// This function draws a PNG image from the SD card to the screen
void drawPNG(const char *filename, bool _wallpaper) {
    fastMode(true);

    File file = SD.open(filename, FILE_READ);
    if (!file) {
        Serial.println("Failed to open file");
        return;
    }

    int rc = png.open(filename, pngOpen, pngClose, pngRead, pngSeek, pngDraw);
    if (rc == PNG_SUCCESS) {
        iswallpaper = _wallpaper;

        if (_wallpaper) {
            free(wallpaper);
            wallpaper = nullptr;
        }

        wallpaper = (uint8_t *)ps_malloc(240 * 294 * sizeof(uint16_t));

        if (!wallpaper) {
            Serial.println("Failed to allocate wallpaper buffer!");
            png.close();
            file.close();
            fastMode(false);
            return;
        }

        Serial.printf("Image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());

        rc = png.decode(NULL, 0);
        png.close();
    } else {
        Serial.printf("PNG open failed: %d\n", rc);
    }

    file.close();
    fastMode(false);
}

// ## Downscale and draw image
// This function was used but was removed because of too long render time
// still can be used in future to scale down images(?)
void drawFromSdDownscale(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, int scale, String file_path) {
    // Enable fast mode for TFT display
    fastMode(true);

    // Open the file from the SD card
    File file = SD.open(file_path);
    if (!file.available()) {
        sysError("SD_CARD_NOT_AVAILABLE");
        return;
    }
    file.seek(pos);

    // Calculate downscaled dimensions
    const int downscaled_width  = size_x / scale;
    const int downscaled_height = size_y / scale;

    // Allocate buffer for the full image (16-bit color, 2 bytes per pixel)
    const int image_size   = size_x * size_y * 2;
    uint8_t  *image_buffer = (uint8_t *)malloc(image_size);
    if (!image_buffer) {
        sysError("MEMORY_ALLOCATION_FAILED");
        file.close();
        fastMode(false);
        return;
    }

    // Read the entire image into the buffer
    if (file.read(image_buffer, image_size) != image_size) {
        sysError("FILE_READ_ERROR");
        free(image_buffer);
        file.close();
        fastMode(false);
        return;
    }
    file.close();

    // Allocate buffer for the downscaled image
    uint16_t *downscaled_buffer = (uint16_t *)malloc(downscaled_width * downscaled_height * sizeof(uint16_t));
    if (!downscaled_buffer) {
        sysError("MEMORY_ALLOCATION_FAILED");
        free(image_buffer);
        fastMode(false);
        return;
    }

    // Downscale the image
    for (int y = 0; y < downscaled_height; y++) {
        for (int x = 0; x < downscaled_width; x++) {
            // Calculate the position in the original image buffer
            int src_x     = x * scale;
            int src_y     = y * scale;
            int src_index = (src_y * size_x + src_x) * 2;

            // Combine two bytes into a 16-bit color value
            uint16_t pixel = (image_buffer[src_index] << 8) | image_buffer[src_index + 1];

            // Store the pixel in the downscaled buffer
            downscaled_buffer[y * downscaled_width + x] = pixel;
        }
    }
    Serial.printf("Y:%d\n", downscaled_height);
    // Push the downscaled image to the screen
    tft.pushImage(pos_x, pos_y, downscaled_width, downscaled_height, downscaled_buffer);

    // Free allocated memory
    free(image_buffer);
    free(downscaled_buffer);

    // Disable fast mode
    fastMode(false);
}

int8_t _signal = 0;
int8_t charge  = 0;

// ## Draw status bar
void drawStatusBar(bool force) {

    sBarChanged += force;
    time(&systemTime);
    tm sbtime = *gmtime(&systemTime);
    if (sbtime.tm_min != systemTimeInfo.tm_min) {
        // Serial.printf("\nSBTIME MIN %d , STI MIN %d\n", sbtime.tm_min, systemTimeInfo.tm_min);
        SaveTime(systemTime);
        sBarChanged = true;
    }

    if (sBarChanged) {
        int  viewport_x     = tft.getViewportX();
        int  viewport_y     = tft.getViewportY();
        int  viewport_w     = tft.getViewportWidth();
        int  viewport_h     = tft.getViewportHeight();
        bool viewport_datum = tft.getViewportDatum();
        tft.resetViewport();
        
        TFT_eSprite _sprite = TFT_eSprite(&tft);

        _sprite.createSprite(240, 26);

        sBarChanged = false;
        charge      = getChargeLevel();

        drawFromSd(0, 0, STATUSBAR_IMAGE, true, _sprite); // statusbar
        drawFromSd(0, 0, SIGNAL_IMAGES[_signal], true, _sprite);
        drawFromSd(207, 0, BATTERY_IMAGES[charge], true, _sprite);
        //  tft.print(String(charge) + String("%"));
        changeFont(1, true, _sprite);
        _sprite.setTextSize(1);
        _sprite.setTextColor(TFT_LIGHTGREY);
        _sprite.setCursor(102, 19);
        _sprite.printf("%02d:%02d", sbtime.tm_hour, sbtime.tm_min);
        if (isScreenLocked) {
            changeFont(0, true, _sprite);
            _sprite.setCursor(0, 0);
            _sprite.setTextSize(1);
            _sprite.setTextColor(TFT_WHITE);
            _sprite.print("KEYBOARD IS LOCKED HOLD * TO UNLOCK");
        }
        _sprite.pushSprite(0, 0);
        _sprite.deleteSprite();
        tft.setViewport(viewport_x, viewport_y, viewport_w, viewport_h, viewport_datum);
    }
}

// ## Draw cutout from SD card
void drawCutoutFromSd(SDImage image,
                      int cutout_x, int cutout_y,
                      int cutout_width, int cutout_height,
                      int display_x, int display_y,
                      String file_path) {
    fastMode(true);

    // Open the file from the SD card
    File file = SD.open(file_path);
    if (!file || !file.available()) {
        sysError("FILE_NOT_AVAILABLE");
        return;
    }

    int image_width = image.w;

    // Calculate the starting offset for the cutout in the file
    uint32_t start_offset = image.address + (cutout_y * image_width + cutout_x) * 2;

    const int buffer_size = cutout_width * 2; // 2 bytes per pixel (16-bit color)
    uint8_t   buffer[buffer_size];

    for (int row = 0; row < cutout_height; row++) {
        // Calculate the offset for the current row
        uint32_t row_offset = start_offset + (row * image_width * 2);
        file.seek(row_offset);

        // Read the row of pixel data into the buffer
        int bytesRead = file.read(buffer, buffer_size);
        if (bytesRead != buffer_size) {
            Serial.println("Error reading row from SD card.");
            return;
        }
        tft.pushImage(display_x, display_y + row, cutout_width, 1, (uint16_t *)buffer);
    }
    file.close();
    fastMode(false);
}

// ## Render menu
//  This function renders the menu icons on the screen.
void rendermenu(int &choice, int old_choice) {

    if (choice > 3)
        choice = 0;
    if (choice < 0)
        choice = 3;
    const struct {
        int x, y;
    } IconPositions[] = {
        {49, 43 + 26}, // Off Icons
        {138, 42 + 26},
        {49, 122 + 26},
        {137, 123 + 26},
        {51, (45 + 26)}, // On Icons
        {141, (44 + 26)},
        {52, (125 + 26)},
        {140, (125 + 26)}};

    int offIndex = old_choice;

    int onIndex = choice;
    if (old_choice != choice)
        drawFromSd(
            IconPositions[offIndex].x,
            IconPositions[offIndex].y,
            MENU_OFF_ICONS[offIndex]);

    drawFromSd(
        IconPositions[onIndex + 4].x,
        IconPositions[onIndex + 4].y,
        MENU_ON_ICONS[onIndex]);
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

TFT_eSprite sprite = TFT_eSprite(&tft);

// ## Draw image from SD card
//  This function draws an image from the SD card to the screen or a sprite buffer.
void drawFromSd(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, bool is_screen_buffer, TFT_eSprite &sbuffer, String file_path, bool transp, uint16_t tc) {
    fastMode(true);
    if (file_path != resPath || !resources) {
        ESP_LOGI("DRAWING", "resource path is different from default or resources not available");
        #ifndef PSRAM_ENABLED
        ESP_LOGW("DRAWING", "PSRAM not available");
        #endif
        File file = SD.open(file_path, FILE_READ);
        if (!file.available())
            sysError("FILE_NOT_AVAILABLE");
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
        file.close();
    } else {
        pos -= RESOURCE_ADDRESS;
        // Serial.printf("POSITION %d\n", pos);

        if (resources) {

            uint16_t *imgData = (uint16_t *)(resources + (pos & ~1));
            if (!transp) {

                if (is_screen_buffer)
                    sbuffer.pushImage(pos_x, pos_y, size_x, size_y, imgData);
                else
                    tft.pushImage(pos_x, pos_y, size_x, size_y, imgData);
            } else {

                if (!sprite.createSprite(size_x, size_y)) {
                    Serial.println("Sprite allocation failed!");
                    return;
                }

                sprite.pushImage(0, 0, size_x, size_y, imgData);

                if (is_screen_buffer)
                    sprite.pushToSprite(&sbuffer, pos_x, pos_y, tc);
                else
                    sprite.pushSprite(pos_x, pos_y, tc);

                sprite.deleteSprite();
            }
        }
    }

    fastMode(false);
}

void drawFromSd(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, String file_path, bool transp, uint16_t tc) {
    drawFromSd(pos, pos_x, pos_y, size_x, size_y, false, screen_buffer, file_path, transp, tc);
}
void drawFromSd(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, bool transp, uint16_t tc) {
    drawFromSd(pos, pos_x, pos_y, size_x, size_y, false, screen_buffer, resPath, transp, tc);
}
void drawFromSd(int x, int y, SDImage sprite, bool is_screen_buffer, TFT_eSprite &sbuffer) {
    if (sprite.address != 0)
        drawFromSd(sprite.address, x, y, sprite.w, sprite.h, is_screen_buffer, sbuffer, resPath, sprite.transp, sprite.tc);
}
void drawFromSd(int x, int y, SDImage sprite) {
    if (sprite.address != 0)
        drawFromSd(sprite.address, x, y, sprite.w, sprite.h, sprite.transp, sprite.tc);
}

// ## Write custom font
//  This function writes a custom font to the screen.
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
void listMenu_header(int type, String title, int page, int pages, int y, bool update) {
    drawFromSd(0, y, BLUEBAR_IMAGE, true);
    drawFromSd(0, y, BLUEBAR_ICONS[type], true);
    screen_buffer.setTextColor(0xFFFF);
    changeFont(1, 1);
    screen_buffer.setCursor(28, y + 19);
    screen_buffer.print(title);
    if (pages > 1) {
        changeFont(0, 1);
        screen_buffer.setCursor(210, y + 15);
        screen_buffer.printf("%d/%d", page + 1, pages);
    }
}

void listMenu_entry(int lindex, int x, int y, mOption choice, int esize, bool lines, bool selected, bool unselected) {
    uint16_t color_active = 0xFDD3;

    int yy = (lindex * esize) + y;

    if (selected)
        screen_buffer.fillRect(0, yy, 240, esize, color_active);
    else if (unselected)
        drawFromSd(0, yy, SDImage(BACKGROUND_IMAGE.address + (yy * 240 * 2), 240, esize), true);
    drawFromSd(x - choice.icon.w, yy, choice.icon, true);

    if (lines) {
        screen_buffer.drawLine(0, yy, 240, yy, 0);
        screen_buffer.drawLine(0, yy + esize, 240, yy + esize, 0);
    }
    screen_buffer.setTextColor(0);
    screen_buffer.setCursor(x + 3, yy + 17);
    changeFont(1, 1);
    screen_buffer.print(choice.label);
}

int listMenu(mOption *choices, int icount, bool lines, int type, String label, bool forceIcons, int findex) {
    screen_buffer.setTextWrap(false, false);
    /*

    int icount count of entries

    label = Title of Menu

    bool lines will it render lines entries

    int type
    0 = MESSAGES
    1 = CONTACTS
    2 = SETTINGS

    */
    fastMode(true);

    screen_buffer.createSprite(240, 294);
    screen_buffer.setTextSize(1);
    screen_buffer.setTextColor(0);

    changeFont(2, 1);
    int selected     = 0;
    int page         = 0;
    int pages        = 0;
    int y            = 0;
    int ly           = 25;
    int x            = 10;
    int old_selected = 0;
    drawFromSd(0, y + 25, SDImage(BACKGROUND_IMAGE.address + 0x2EE0, 240, 269), true);

    if (icount == 0) {
        listMenu_header(type, label, 0, 0, y, false);
        screen_buffer.setTextColor(0);
        changeFont(1, 1);
        screen_buffer.setCursor(75, 45);
        screen_buffer.print("< Empty >");
        screen_buffer.pushSprite(0, 26);
        screen_buffer.deleteSprite();
        while (buttonsHelding() != BACK)
            ;
        return -1;
    }
    int entry_size = screen_buffer.fontHeight();
    if (choices[0].icon.h > entry_size) {
        entry_size = choices[0].icon.h;
    }
    x += choices[0].icon.w;
    int per_page = 269 / entry_size;
    pages        = (icount + per_page - 1) / per_page;

    listMenu_header(type, label, page, pages, y, false);
    int startIndex = page * per_page;
    int endIndex   = std::min(startIndex + per_page, icount);

    for (int i = 0; i < (endIndex - startIndex); i++) {
        listMenu_entry(i, x, y + ly, choices[startIndex + i], entry_size, lines, false, false);
    }
    listMenu_entry(selected, x, y + ly, choices[selected + (page * per_page)], entry_size, lines, true, false);
    screen_buffer.pushSprite(0, 26);
    bool exit                = false;
    int  total_items_on_page = 0; // Move declaration outside the switch statement
    while (!exit) {
        int c = buttonsHelding();

        switch (c) {
        case SELECT:
            screen_buffer.deleteSprite();
            fastMode(false);
            return selected + (page * per_page);
            break;
        case BACK:
            screen_buffer.deleteSprite();
            return -1;
            break;

        case UP:
            old_selected = selected;
            selected--;
            fastMode(true);
            if (selected < 0) {
                if (page > 0) {

                    page--;
                    selected = per_page - 1;
                } else {

                    page     = pages - 1;
                    selected = (icount % per_page == 0) ? per_page - 1 : (icount % per_page) - 1;
                }

                drawFromSd(0, y + 25, SDImage(BACKGROUND_IMAGE.address + 0x2EE0, 240, 269), true);
                listMenu_header(type, label, page, pages, y, true);

                int startIndex = page * per_page;
                int endIndex   = std::min(startIndex + per_page, icount);

                for (int i = 0; i < (endIndex - startIndex); i++) {
                    listMenu_entry(i, x, y + ly, choices[startIndex + i], entry_size, lines, false, false);
                }
            } else {

                listMenu_entry(old_selected, x, y + ly, choices[old_selected + (page * per_page)], entry_size, lines, false, true);
            }

            listMenu_entry(selected, x, y + ly, choices[selected + (page * per_page)], entry_size, lines, true, false);
            screen_buffer.pushSprite(0, 26);

            fastMode(false);
            break;

        case DOWN:
            fastMode(true);
            old_selected = selected;
            selected++;

            total_items_on_page = std::min(per_page, icount - (page * per_page)); // Update value here

            if (selected >= total_items_on_page) {
                if (page < pages - 1) {

                    page++;
                    selected = 0;
                } else {
                    page     = 0;
                    selected = 0;
                }

                drawFromSd(0, y + ly, SDImage(BACKGROUND_IMAGE.address + 0x2EE0, 240, 269), true);
                listMenu_header(type, label, page, pages, y, true);
                int startIndex = page * per_page;
                int endIndex   = std::min(startIndex + per_page, icount);

                for (int i = 0; i < (endIndex - startIndex); i++) {
                    listMenu_entry(i, x, y + ly, choices[startIndex + i], entry_size, lines, false, false);
                }
            } else {

                listMenu_entry(old_selected, x, y + ly, choices[old_selected + (page * per_page)], entry_size, lines, false, true);
            }

            listMenu_entry(selected, x, y + ly, choices[selected + (page * per_page)], entry_size, lines, true, false);

            screen_buffer.pushSprite(0, 26);

            fastMode(false);
            break;
        case RIGHT:
            if (pages > 1) {
                page     = (page + 1) % pages;
                selected = 0;
                drawFromSd(0, y + 25, SDImage(BACKGROUND_IMAGE.address + 0x2EE0, 240, 269), true);
                listMenu_header(type, label, page, pages, y, true);

                int startIndex = page * per_page;
                int endIndex   = std::min(startIndex + per_page, icount);

                for (int i = 0; i < (endIndex - startIndex); i++) {
                    listMenu_entry(i, x, y + ly, choices[startIndex + i], entry_size, lines, false, false);
                }
                listMenu_entry(selected, x, y + ly, choices[selected + (page * per_page)], entry_size, lines, true, false);
                screen_buffer.pushSprite(0, 26);
            }
            break;
        case LEFT:
            if (pages > 1) {
                page     = (page - 1 + pages) % pages;
                selected = 0;
                drawFromSd(0, y + 25, SDImage(BACKGROUND_IMAGE.address + 0x2EE0, 240, 269), true);
                listMenu_header(type, label, page, pages, y, true);

                int startIndex = page * per_page;
                int endIndex   = std::min(startIndex + per_page, icount);

                for (int i = 0; i < (endIndex - startIndex); i++) {
                    listMenu_entry(i, x, y + ly, choices[startIndex + i], entry_size, lines, false, false);
                }
                listMenu_entry(selected, x, y + ly, choices[selected + (page * per_page)], entry_size, lines, true, false);
                screen_buffer.pushSprite(0, 26);
            }
            break;
        }
    }
    screen_buffer.deleteSprite();
    return -1;
}

// Converter from old type of listMenu to new
int listMenu(const String choices[], int icount, bool images, int type, String label, bool forceIcons, int findex) {
    mOption *optionArr = new mOption[icount];
    for (int i = 0; i < icount; i++) {
        optionArr[i].label = choices[i];
        optionArr[i].icon  = SDImage();
    }
    return listMenu(optionArr, icount, images, type, label, forceIcons, findex);
}

int lmng_offset = 0;
// Render entry of listMenuNonGraphical()
void renderlmng(mOption *choices, int x, int y, int icount, String label, int index, uint16_t color_active, uint16_t color_inactive) {
    uint max_per_page = ((320 - y - tft.fontHeight()) / tft.fontHeight());

    if (index >= max_per_page + lmng_offset) {
        tft.fillRect(x, y, 240 - x, 320 - y, 0);
        tft.fillScreen(0);
    } else if (index < lmng_offset) {
        lmng_offset = index;
        tft.fillRect(x, y, 240 - x, 320 - y, 0);
    }
    tft.setCursor(x, y);
    tft.setTextColor(color_inactive);
    tft.println(label + "  ");

    for (int i = lmng_offset; i < max_per_page + lmng_offset; i++) {
        if (i == index)
            tft.setTextColor(color_active);
        else
            tft.setTextColor(color_inactive);
        if (i < icount)
            tft.println("  " + choices[i].label);
    }
}

// listMenu that doesn't use graphics
int listMenuNonGraphical(mOption *choices, int icount, String label, int y) {
    suspendCore(true);
    int x     = 0;
    int index = 0;
    changeFont(0);
    tft.fillRect(x, y, 240 - x, 320 - y, 0);
    // uint     max_per_page   = ((320 - y - tft.fontHeight()) / tft.fontHeight());
    uint16_t color_active   = TFT_RED;
    uint16_t color_inactive = TFT_WHITE;
    tft.setTextColor(color_inactive);
    renderlmng(choices, x, y, icount, label, index, color_active, color_inactive);

    bool exit = false;
    while (!exit) {
        int c = buttonsHelding(false);
        switch (c) {
        case UP:
            index--;
            if (index < 0)
                index = icount - 1;
            renderlmng(choices, x, y, icount, label, index, color_active, color_inactive);
            break;
        case DOWN:
            index++;
            if (index >= icount)
                index = 0;
            renderlmng(choices, x, y, icount, label, index, color_active, color_inactive);
            break;
        case SELECT:
            suspendCore(false);
            return index;
            break;
        case BACK:
            suspendCore(false);
            return -1;
            break;
        }
    }

    suspendCore(false);
    return -1;
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

int choiceMenu(const String choices[], int count, bool context) {
    int      x              = 30;
    int      y              = 95;
    int      mul            = 20;
    uint16_t color_active   = 0xF9C0;
    uint16_t color_inactive = 0x0000;

    if (context) {
        drawFromSd(16, 100, CONTEXT_MENU_IMAGE);
        x = 40;
        y = 120;
    } else {
        drawFromSd(0, 68, WHITE_BOTTOM_IMAGE);
        x = 30;
        y = 95;
    }


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

                drawFromSd(16, 100, CONTEXT_MENU_IMAGE);
                for (int i = 0; i < count; i++) {
                    tft.setCursor(x, y + (mul * i));
                    tft.print(choices[i]);
                }
            } else {
                tft.setTextSize(1);
                tft.setTextColor(color_inactive);

                drawFromSd(0, 68, WHITE_BOTTOM_IMAGE);
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

                drawFromSd(16, 100, CONTEXT_MENU_IMAGE);
                for (int i = 0; i < count; i++) {
                    tft.setCursor(x, y + (mul * i));
                    tft.print(choices[i]);
                }
            } else {
                tft.setTextSize(1);
                tft.setTextColor(color_inactive);

                drawFromSd(0, 68, WHITE_BOTTOM_IMAGE);
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
    }
    return -1;
}

void findSplitPosition(String text, int charIndex, int &posX, int &posY, int direction) {
    // int prevNL       = 0; // Last newline position
    int lastNewLine  = 0; // Current line's start position
    int curPosInText = 0; // Current character position
    posX             = 0; // Reset position to 0 at the start
    posY             = 0; // Reset vertical position

    // Loop through text up to the charIndex
    for (; curPosInText < charIndex; curPosInText++) {
        posX = tft.textWidth(text.substring(lastNewLine, curPosInText + 1));

        // If the position exceeds 240px width or we encounter a newline
        if (posX >= 240 || text[curPosInText] == '\n') {
            posX = 0; // Reset X position for the next line
            // prevNL      = lastNewLine;      // Remember the start of the previous line
            lastNewLine = curPosInText + 1; // Set new line's start
            posY += tft.fontHeight();       // Move to the next line vertically
        }

        if (curPosInText >= text.length() - 1) {
            break; // Stop if we're at the end of the text
        }
    }
}

// Find the character index in the text based on the current position and direction
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

// Split a string into lines based on the screen width
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

uint8_t *wallpaper = nullptr;
void     drawWallpaper() {
    if (!isSPIFFS && SD.exists(resPath)) {
        if (!wallpaper) {
            if (wallpaperIndex >= 0 && wallpaperIndex < 42) {
                loadResource(WALLPAPER_IMAGES_BASE.address + WALLPAPER_IMAGES_MULTIPLIER * wallpaperIndex, resPath, &wallpaper, WALLPAPER_IMAGES_BASE.w, WALLPAPER_IMAGES_BASE.h);
            } else {
                if (SD.exists(currentWallpaperPath)) {
                    drawPNG(currentWallpaperPath.c_str(), true);
                } else {
                    wallpaperIndex = 0;
                    loadResource(WALLPAPER_IMAGES_BASE.address + WALLPAPER_IMAGES_MULTIPLIER * wallpaperIndex, resPath, &wallpaper, WALLPAPER_IMAGES_BASE.w, WALLPAPER_IMAGES_BASE.h);
                }
            }
            tft.pushImage(0, 26, 240, 294, (uint16_t *)wallpaper);
        } else {
            tft.pushImage(0, 26, 240, 294, (uint16_t *)wallpaper);
            Serial.println("WALLPAPER");
        }
    } else {
        if (SD.exists(currentWallpaperPath)) {
            drawPNG(currentWallpaperPath.c_str(), true);
        } else {
            uint16_t colora = random(0, 0x7777);
            uint16_t colorb = random(0, 0x7777);
            if (random(0, 1000) % 2 == 0)
                tft.fillRectVGradient(0, 26, 240, 294, colora, colorb);
            else
                tft.fillRectHGradient(0, 26, 240, 294, colora, colorb);
            changeFont(0);
            tft.setTextSize(1);
            tft.setTextColor(~colora);
            tft.setCursor(0, 26);
            tft.println("NO SDCARD OR RESOURCE FILE.\nBUILT-IN WALLPAPERS WILL NOT BE VISIBLE");
        }
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