#include "Input.h"
#include "../Platform/ESPPlatform.h"
#include "../Platform/ard_esp.h"
#ifdef PC
#include <SDL2/SDL.h>
#endif
const char *ITAG      = "INPUT";
int         millDelay = 0;

// Function to handle the idle state
void idle() {

    if (hw->millis() > millSleep + (delayBeforeSleep / 2) &&
        hw->millis() < millSleep + delayBeforeSleep) {
        setBrightness(brightness * 0.1);
        fastMode(false);
    }
    else if (hw->millis() > millSleep + delayBeforeSleep) { setBrightness(0); }
    else {
        setBrightness(brightness);
        fastMode(true);
    }
    if (hw->millis() > millSleep + delayBeforeSleep + delayBeforeLock && !isScreenLocked) {
        LockScreen();
    }

    drawStatusBar(false);

    checkVoiceCall();
    delay(50);
    if (hw->millis() - millDelay > DBC_MS) {
        millDelay = hw->millis();
        drawStatusBar();
    }
}

/*
 * Number input field (used on Main screen)
 * @param first first number to be displayed (since it being called by button press)
 */

void numberInput(char first) {
    tft.fillRect(0, 300, 240, 20, 0);
    sBarChanged = true;
    drawStatusBar();
    const uint8_t max_char = 13;
    NString       number;
    number += first;
    char c = 127;
    tft.setTextColor(TFT_WHITE);
    changeFont(0);
    tft.setTextSize(3);
    tft.fillRect(0, 300, 240, 20, 0);
    tft.setCursor(0, 300);
    tft.print(number);
    while (true) {
        while (c == 127 || c < 0) { c = buttonsHelding(); }

        switch (c) {
        case ANSWER:
            if (!number.isEmpty()) { makeCall(Contact("", number)); }

            return;
        case LEFT:
            number.remove(number.length() - 1);
            tft.fillRect(0, 300, 240, 20, 0);
            tft.setCursor(0, 300);
            tft.print(number);

            break;
        case BACK: return;
        default: break;
        }

        if ((c >= '0' || c == '*' || c == '#') && c <= '9' && number.length() < max_char) {
            if (!simIsBusy) { sendATCommand("AT+CLDTMF=15,\"" + NString(char(c)) + "\",10", 1); }
            number += c;
            tft.fillRect(0, 300, 240, 20, 0);
            tft.setCursor(0, 300);
            tft.print(number);
            c = 255;
        }

        ESP_LOGI(ITAG, "%d", c);
        c = 255;
    }
}

/*
 * Show character selection screen
 * @param text text to be displayed
 * @param pos selected character position
 */
void showText(const char *text, int pos) {

    int  h        = tft.getViewportHeight();
    int  w        = tft.getViewportWidth();
    int  vx       = tft.getViewportX();
    int  vy       = tft.getViewportY();
    bool viewport = false;
    if (tft.getViewportHeight() < 320) {
        tft.resetViewport();
        viewport = true;
    }

    int pfont     = currentFont;
    int textColor = tft.textcolor;
    int textSize  = tft.textsize;

    changeFont(0);
    tft.setTextSize(2);
    tft.setCursor(0, INPUT_LOCATION_Y);

    for (int i = 0; i < (int)(strchr(text, '\r') - text); i++) {
        if (i != pos) { tft.setTextColor(0xFFFF, 0, true); }
        else { tft.setTextColor(0xFFFF, 0x001F, true); }
        if (text[i] == '\n') { tft.print("NL"); }
        else if (text[i] == '\b') { tft.print("<-"); }
        else { tft.print(text[i]); }
    }

    tft.textcolor = textColor;
    changeFont(pfont);
    tft.setTextSize(textSize);
    if (viewport) { tft.setViewport(vx, vy, w, h); }
}

/*
 * Character selection
 * @param input pressed button
 * @param onlynumbers if true, only numbers are allowed
 * @param nonl disable new line
 * @return selected character
 */
char textInput(int input, bool onlynumbers, bool nonl, bool dontRedraw, int *retButton) {
    if (input == -1) { return 0; }
    char buttons[12][12] = {" \b0+@\n\r", "1,.?!()\r",   "2ABCabc\r", "3DEFdef\r",
                            "4GHIghi\r",  "5JKLjkl\r",   "6MNOmno\r", "7PQRSpqrs\r",
                            "8TUVtuv\r",  "9WXYZwxyz\r", "*\r",       "#\r"};

    if (nonl) { buttons[0][5] = '\r'; }
    if (onlynumbers) {
        buttons[0][2] = '\r';
        for (int i = 1; i < 12; i++) { buttons[i][1] = '\r'; }
    }
    bool first = true;
    // int  sizes[12];
    char result       = 0;
    int  pos          = 0;
    int  currentIndex = input >= '0' && input <= '9' ? input - 48
                        : input == '*'               ? 10
                        : input == '#'               ? 11
                                                     : -1;
    if (currentIndex == -1) {
        ESP_LOGI(ITAG, "%s", NString("UNKNOWN BUTTON:" + NString(input)).c_str());
        return 0;
    }

    for (int i = 0; i < 12; i++) {
        int b = 0;
        for (; b < 12; b++) {
            if (buttons[i][b] == '\r') { break; }
        }
        b = 0;
    }
    int mil  = hw->millis();
    pos      = -1;
    int curx = tft.getCursorX();
    int cury = tft.getCursorY();
    while (hw->millis() - mil < DIB_MS) {
        curx = tft.getCursorX();
        cury = tft.getCursorY();
        // ESP_LOGI(ITAG,"POSITION:" + NString(pos));
        int c = buttonsHelding();

        if (c == input || first) {
            if (pos < (int)(strchr(buttons[currentIndex], '\r') - buttons[currentIndex])) {
                mil = hw->millis();
                pos++;
                result = buttons[currentIndex][pos];
                showText(buttons[currentIndex], pos);
                tft.setCursor(curx, cury);
            }
            else {
                mil    = hw->millis();
                pos    = 0;
                result = buttons[currentIndex][pos];
                showText(buttons[currentIndex], pos);
                tft.setCursor(curx, cury);
            }
        }
        if (c != input && c != -1) {
            mil = DIB_MS + 1;
            if (!retButton) { *retButton = c; }
        }
        first = false;
    }

    tft.setCursor(curx, cury);
    bool viewport = false;
    int  h        = tft.getViewportHeight();
    int  w        = tft.getViewportWidth();
    int  vx       = tft.getViewportX();
    int  vy       = tft.getViewportY();

    if (tft.getViewportHeight() < 320) {
        tft.resetViewport();
        viewport = true;
    }
    if (!dontRedraw) {
        res.DrawImage(R_LIST_MENU_BACKGROUND, 0, {0, INPUT_LOCATION_Y - 51}, {0, INPUT_LOCATION_Y},
                      {0, INPUT_LOCATION_Y});
        // drawCutoutFromSd(SDImage(0x639365, 240, 269, 0, false), 0, INPUT_LOCATION_Y - 51, 120,
        // 20, 0, INPUT_LOCATION_Y);
        // TODO
    }
    if (viewport) { tft.setViewport(vx, vy, w, h); }
    if (result == '\r') { return 0; }
    return result;
}

int lastresult = -1;
/*
 * Returns pressed button
 * @param _idle execute background `idle()` function if true
 */
int buttonsHelding(bool _idle) {

    /*
     * SIDE BUTTON 10
     *  7 8 9
     * |M|I|B||
     *      ^=W
     * <=A  0=SP >=D
     * A=Q  V=S  D=E
     * 1   2   3
     * 4   5   6
     * 7   8   9
     * *   0   #
     *
     * A-ANSWER
     * D-DECLINE
     * 0-SELECT
     *
     * because we don't have BACK button Button D (Decline) also acts as BACK
     *
     *
     */

#ifdef PC
    if (currentRenderTarget && currentRenderTarget->getType() == RENDER_TARGET_TYPE_SCREEN) {
        currentRenderTarget->present();
    }
#endif
    if (_idle) { idle(); }
    char input = 0;
    input      = hw->getCharInput();
    int result = hw->getKeyInput();

    if (lastresult != result) { millSleep = hw->millis(); }

    if (result != 0) { while (result == hw->getKeyInput() && hw->millis() - millSleep < 1500); }

    lastresult = result;

    // Serial control support
    // You can control device keypad from other device through Serial port

    if (Serial.available() || input != 0) {
#ifndef PC
        input = Serial.read();
#endif
        millSleep = hw->millis();
        switch (input) {
        case 'a':
            ESP_LOGI(ITAG, "LEFT");
            result = 4;
            break;
        case 'd':
            ESP_LOGI(ITAG, "RIGHT");
            result = 6;
            break;
        case 'w':
            ESP_LOGI(ITAG, "UP");
            result = 2;
            break;
        case 's':
            ESP_LOGI(ITAG, "DOWN");
            result = 8;
            break;
        case ' ':
            ESP_LOGI(ITAG, "SELECT");
            result = 5;
            break;
        case 'e':
            ESP_LOGI(ITAG, "ANSWER");
            result = 7;
            break;
        case 'q':
            ESP_LOGI(ITAG, "BACK");
            result = 9;
            break;
        case '*':
            ESP_LOGI(ITAG, "%c", input);
            result = 19;
            break;
        case '#':
            ESP_LOGI(ITAG, "%c", input);
            result = 21;
            break;
        case 'l': ESP_LOGI(ITAG, "Restart");
#ifndef PC
            ESP.restart();
#endif
            break;
        default:
            if (input >= '0' && input <= '9') {
                ESP_LOGI(ITAG, "%c", input);
                result += 10;
                result -= '1';
            }
            break;
        }
    }
    switch (result) {
    case 2: return UP;
    case 4: return LEFT;
    case 5: return SELECT;
    case 6: return RIGHT;
    case 7: return ANSWER;
    case 8: return DOWN;
    case 9: return DECLINE;
    case 19: return '*';
    case 20: return '0';
    case 21: return '#';
    default:
        if (result > 9 && result < 19) {
            result -= 10;
            result += '1';
            return result;
        }
        break;
    }

    return -1;
}