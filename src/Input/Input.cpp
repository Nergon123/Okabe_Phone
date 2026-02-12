#include "Input.h"

#ifdef PC
#include <SDL2/SDL.h>
#endif
const char *ITAG      = "INPUT";
int         millDelay = 0;
int         idleDelay = 0;

// Function to handle the idle state
void idle() {

    if (NI_delay(idleDelay, 50)) { return; }
    idleDelay = hw->millis();
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

    const uint8_t max_length = 13;
    NString       number;
    number += first;

    int c = 127;

    tft.setTextColor(TFT_WHITE);
    changeFont(0);
    tft.setTextSize(3);

    auto redraw = [&]() {
        tft.fillRect(0, 300, 240, 20, 0);
        tft.setCursor(0, 300);
        tft.print(number);
        currentRenderTarget->present();
    };

    redraw();

    while (true) {
        while (c == 127 || c == -1) { c = buttonsHelding(); }

        switch (c) {

        case ANSWER:
            if (!number.isEmpty()) { makeCall(Contact("", number)); }
            return;

        case LEFT:
            if (!number.isEmpty()) {
                number.remove(number.length() - 1);
                redraw();
            }
            break;

        case BACK: return;

        default:
            if ((std::isdigit(static_cast<unsigned char>(c)) || c == '*' || c == '#') &&
                number.length() < max_length) {
                if (!simIsBusy) {
                    sendATCommand("AT+CLDTMF=15,\"" + NString(char(c)) + "\",10", 1);
                }

                number += char(c);
                redraw();
            }
            break;
        }
        c = 127; // reset key
    }
}

/*
 * Show character selection screen
 * @param text text to be displayed
 * @param pos selected character position
 */
void showText(const char *text, int pos) {
    Viewport      vp       = tft.getViewport();
    bool          viewport = false;
    RenderTarget *before   = tft.activeRenderTarget;
    tft.setRenderTarget(currentRenderTarget);
    if (vp.h < 320) {
        tft.resetViewport();
        viewport = true;
    }

    int pfont     = currentFont;
    int textColor = tft.textcolor;
    int textSize  = tft.textsize;

    changeFont(0);
    tft.setTextSize(2);
    tft.setCursor(0, INPUT_LOCATION_Y);

    for (int i = 0; text[i] != 0; i++) {
        if (i != pos) { tft.setTextColor(0xFFFF, 0, true); }
        else { tft.setTextColor(0xFFFF, 0x001F, true); }
        if (text[i] == '\n') { tft.print("NL"); }
        else if (text[i] == '\b') { tft.print("<-"); }
        else { tft.print(text[i]); }
    }

    tft.setTextColor(textColor);
    changeFont(pfont);
    tft.setTextSize(textSize);
    if (viewport) { tft.setViewport(vp); }
    tft.setRenderTarget(before);
    currentRenderTarget->present();
}

/*
 * Character selection
 * @param input pressed button
 * @param onlynumbers if true, only numbers are allowed
 * @param nonl disable new line
 * @return selected character
 */
NString textInput(int input, uint8_t useCharset, bool nonl, bool dontRedraw, int *retButton) {
    if (input == -1) { return 0; }
    currentRenderTarget->setUseBuffer(false);
    char buttons[5][10][32] = {
        {" 0+\n",          ".,?'\"1-()@/:_",       "abcà2",          "defèé3",       "ghiì4",
         "jkl5",           "mnoò6",                "pqrs7",          "tuvù8",        "wxyz9"},
        {" 0+\n",          ".,?'\"1-()@/:_",       "ABCÀ2",          "DEFÈÉ3",       "GHIÌ4",
         "JKL5",           "MNOÒ6",                "PQRS7",          "TUVÙ8",        "WXYZ9"},
        {'0',              '1',                    '2',              '3',            '4',   
         '5',              '6',                    '7',              '8',            '9'},
        {"わをんー～　\n", "あいうえおぁぃぅぇぉ", "かきくけこ",     "さしすせそ",   "たちつてとっ",
         "なにぬねの",     "はひふへほ",           "まみむめも",     "やゆよゃゅょ", "らりるれろ"},
        {"ワヲンー～　\n", "アイウエオァィゥェォ", "カキクケコ",     "サシスセソ",   "タチツテトッ",
         "ナニヌネノ",     "ハヒフヘホ",           "マミムメモ",     "ヤユヨャュョ", "ラリルレロ"}
    };
                                 // * = ﾞﾟ 

    if (nonl) {buttons[SMALL_LATIN][0][3] = '\0'; buttons[CAPS_LATIN][0][3] = '\0'; buttons[HIRAGANA][0][12] = '\0'; buttons[KATAKANA][0][12] = '\0';}
    
    bool first = true;
    // int  sizes[12];
    char result       = 0;
    int  pos          = 0;
    int  currentIndex = input >= '0' && input <= '9' ? input - 48
                        : input == '*'               ? 10
                        : input == '#'               ? 11
                                                     : -1;

    if (currentIndex == -1) {
        ESP_LOGI(ITAG, "UNKNOWN BUTTON:%d", input);
        return 0;
    }

    // for (int i = 0; i < 12; i++) {
    //     int b = 0;
    //     for (; b < 12; b++) {
    //         if (buttons[i][b] == '\0') { break; }
    //     }
    //     b = 0;
    // }
    ulong mil = hw->millis();
    pos       = -1;
    int curx  = tft.getCursorX();
    int cury  = tft.getCursorY();
    while (hw->millis() - mil < DIB_MS) {
        curx  = tft.getCursorX();
        cury  = tft.getCursorY();
        int c = buttonsHelding();

        if (c == input || first) {
            if (pos < (int)(strchr(buttons[useCharset][currentIndex], '\0') - buttons[useCharset][currentIndex])) {
                mil = hw->millis();
                pos++;
                result = buttons[useCharset][currentIndex][pos];
                showText(buttons[useCharset][currentIndex], pos);
                tft.setCursor(curx, cury);
            }
            else {
                mil    = hw->millis();
                pos    = 0;
                result = buttons[useCharset][currentIndex][pos];
                showText(buttons[useCharset][currentIndex], pos);
                tft.setCursor(curx, cury);
            }
        }
        if (c != input && c != -1) {
            mil = DIB_MS + 1;
            if (!retButton) { retButton = new int(c); }
        }
        first = false;
    }

    tft.setCursor(curx, cury);
    Viewport vp       = tft.getViewport();
    bool     viewport = false;
    if (vp.h < 320) {
        tft.resetViewport();
        viewport = true;
    }
    if (viewport) { tft.setViewport(vp); }
    currentRenderTarget->setUseBuffer(true);
    currentRenderTarget->present();
    if (result == '\r') { return 0; }

    return result;
    (void)dontRedraw;
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
     * because we don't have separate BACK button Button D (Decline) also acts as BACK
     *
     *
     */

    if (_idle) { idle(); }
    hw->delay(1); // watchdog feed
    char input = 0;
    input      = hw->getCharInput();
    int result = hw->getKeyInput();

    if (lastresult != result) { millSleep = hw->millis(); }

    if (result != 0) { while (result == hw->getKeyInput() && hw->millis() - millSleep < 1500); }

    lastresult = result;

    // Serial control support
    // You can control device keypad from other device through Serial port

    if (input != 0) {
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
            if (input >= '0' && input <= '9') { return input; }
            break;
        }
    }
    switch (result) {
    //case 1: return LEFTFN;
    case 2: return UP;
    //case 3: return RIGHTFN;
    // Don't mind how the values below are increased by 3, I'm unable to use 4, 5 and 6. I'll change this back before the PR
    case 7: return LEFT;
    case 8: return SELECT;
    case 9: return RIGHT;
    case 10: return ANSWER;
    case 11: return DOWN;
    case 12: return DECLINE;
    case 22: return '*';
    case 23: return '0';
    case 24: return '#';
    default:
        if (result >= 13 && result <= 21) {
            return char('1' + (result - 13));
            break;
        }

        return -1;
    }
}