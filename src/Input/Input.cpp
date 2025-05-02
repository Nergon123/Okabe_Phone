#include "Input.h"

int millDelay = 0;
// Function to handle the idle state
void idle() {

    if (millis() > millSleep + (delayBeforeSleep / 2) && millis() < millSleep + delayBeforeSleep) {
        setBrightness(brightness * 0.1);
    } else if (millis() > millSleep + delayBeforeSleep) {
        setBrightness(0);
    } else {
        setBrightness(brightness);
    }
    if (millis() > millSleep + delayBeforeSleep + delayBeforeLock && !isScreenLocked) {
        LockScreen();
    }

    drawStatusBar(false);

    checkVoiceCall();
    delay(50);
    if (millis() - millDelay > DBC_MS) {
        millDelay = millis();
        drawStatusBar();
    }
}

// check if certain GPIO button is pressed (Deprecated?)
bool checkEXButton(int pin) {
    if (digitalRead(pin) == LOW) {
        delay(50);
        while (digitalRead(pin) == LOW)
            ;
        return true;
    }
    return false;
}

// check which MCP23017 button is pressed
int checkButton() {

    mcp.portMode(MCP23017Port::A, 0xFF);
    mcp.portMode(MCP23017Port::B, 0);
    mcp.writePort(MCP23017Port::B, 0x00);
    uint8_t a = mcp.readPort(MCP23017Port::A);

    mcp.portMode(MCP23017Port::B, 0xFF);
    mcp.portMode(MCP23017Port::A, 0);
    mcp.writePort(MCP23017Port::A, 0x00);
    uint8_t b = mcp.readPort(MCP23017Port::B);

    a = ~a;
    a >>= 1;
    b = ~b;

    uint8_t ar = 0xFF, br = 0xFF;
    for (int i = 0; i < 8; ++i) {
        if (a & (1 << i) && ar == 0xFF)
            ar = i;

        if (b & (1 << i) && br == 0xFF)
            br = i;
    }
    ar++;
    br++;
    // if (br != 0)
    //   br = 3 - br;

    if (ar != 0xFF && br != 0xFF) {
        uint8_t res = ar == 0 && br == 0 ? 0 : 21 - (ar * 3) + br;

        return res;
    }
    return 0;
}

void numberInput(char first) {
    tft.fillRect(0, 300, 240, 20, 0);
    sBarChanged = true;
    drawStatusBar();
    const uint8_t max_char = 13;
    String        number;
    number += first;
    char c = 255;
    tft.setTextColor(TFT_WHITE);
    changeFont(0);
    tft.setTextSize(3);
    tft.fillRect(0, 300, 240, 20, 0);
    tft.setCursor(0, 300);
    tft.print(number);
    DBC_MS = 6000;
    while (true) {
        while (c == 255) {
            c = buttonsHelding();
        }

        switch (c) {
        case ANSWER:
            if (!number.isEmpty())
                makeCall(Contact(number, ""));
            return;
            break;
        case LEFT:

            number.remove(number.length() - 1);

            tft.fillRect(0, 300, 240, 20, 0);
            tft.setCursor(0, 300);
            tft.print(number);
            break;
        case BACK:
            return;
            break;
        default:
            break;
        }

        if ((c >= '0' || c == '*' || c == '#') && c <= '9' && number.length() < max_char) {
            if (!simIsBusy)
                sendATCommand("AT+CLDTMF=15,\"" + String(char(c)) + "\",10", 1);

            number += c;
            tft.fillRect(0, 300, 240, 20, 0);
            tft.setCursor(0, 300);
            tft.print(number);
            c = 255;
        }

        Serial.println(c, DEC);
        c = 255;
    }
    DBC_MS = 1000;
}

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

        if (i != pos)
            tft.setTextColor(0xFFFF, 0, true);
        else
            tft.setTextColor(0xFFFF, 0x001F, true);
        if (text[i] == '\n')
            tft.print("NL");
        else if (text[i] == '\b')
            tft.print("<-");
        else
            tft.print(text[i]);
    }

    tft.textcolor = textColor;
    changeFont(pfont);
    tft.setTextSize(textSize);
    if (viewport)
        tft.setViewport(vx, vy, w, h);
}

char textInput(int input, bool onlynumbers, bool nonl) {
    if (input == -1)
        return 0;
    char buttons[12][12] = {
        " \b0+@\n\r",
        "1,.?!()\r",
        "2ABCabc\r",
        "3DEFdef\r",
        "4GHIghi\r",
        "5JKLjkl\r",
        "6MNOmno\r",
        "7PQRSpqrs\r",
        "8TUVtuv\r",
        "9WXYZwxyz\r",
        "*\r",
        "#\r"};

    // nonl - disable new line
    if (nonl) {
        buttons[0][5] = '\r';
    }
    if (onlynumbers) {
        buttons[0][2] = '\r';
        for (int i = 1; i < 12; i++) {
            buttons[i][1] = '\r';
        }
    }
    bool first = true;
    // int  sizes[12];
    char result = 0;
    int  pos    = 0;
    int  currentIndex =
        input >= '0' && input <= '9'
             ? input - 48
         : input == '*' ? 10
         : input == '#' ? 11
                        : -1;
    if (currentIndex == -1) {
        Serial.println("UNKNOWN BUTTON:" + String(input));
        return 0;
    }

    for (int i = 0; i < 12; i++) {
        int b = 0;
        for (; b < 12; b++) {
            if (buttons[i][b] == '\r')
                break;
        }
        // sizes[i] = b;
        //  Serial.println(b, DEC);
        b = 0;
    }
    int mil  = millis();
    pos      = -1;
    int curx = tft.getCursorX();
    int cury = tft.getCursorY();
    while (millis() - mil < DIB_MS) {
        curx = tft.getCursorX();
        cury = tft.getCursorY();
        // Serial.println("POSITION:" + String(pos));
        int c = buttonsHelding();
        if (c == input || first) {
            if (pos < (int)(strchr(buttons[currentIndex], '\r') - buttons[currentIndex])) {
                mil = millis();
                pos++;
                result = buttons[currentIndex][pos];
                showText(buttons[currentIndex], pos);
                tft.setCursor(curx, cury);
            } else {
                mil    = millis();
                pos    = 0;
                result = buttons[currentIndex][pos];
                showText(buttons[currentIndex], pos);
                tft.setCursor(curx, cury);
            }
        }
        first = false;
    }

    tft.fillRect(0, 300, 240, 30, 0);

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

    drawCutoutFromSd(SDImage(0x639365, 240, 269, 0, false), 0, INPUT_LOCATION_Y - 51, 120, 20, 0, INPUT_LOCATION_Y);
    if (viewport) {
        tft.setViewport(vx, vy, w, h);
    }
    if (result == '\r') {
        return 0;
    }
    return result;
}

int lastresult = -1;
// Returns pressed button
// _idle - execute background function if true
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
    // BACK_BUTTON_PIN = 0;
    // UP_BUTTON_PIN = 38;
    // SELECT_BUTTON_PIN = 37;
    // DOWN_BUTTON_PIN = 39;

    // #ifndef DEVMODE
    //   if (checkButton(0))
    //     return BACK;
    //   if (checkButton(37))
    //     return SELECT;
    //   if (checkButton(38))
    //     return UP;
    //   if (checkButton(39))
    //     return DOWN;
    // #endif
    if (_idle)
        idle();

    int result = checkButton();
    if (lastresult != result)
        millSleep = millis();

    if (result != 0)
        while (result == checkButton() && millis() - millSleep < 1500)
            ;

    lastresult = result;
    // setBrightness(brightness);

    // For some reason serial control support
    // You can control device keypad from other device through Serial port

    if (Serial.available()) {
        char input = Serial.read();
        millSleep  = millis();
        switch (input) {
        case 'a':
            Serial.println("LEFT");
            result = 4;
            break;
        case 'd':
            Serial.println("RIGHT");
            result = 6;
            break;
        case 'w':
            Serial.println("UP");
            result = 2;
            break;
        case 's':
            Serial.println("DOWN");
            result = 8;
            break;
        case ' ':
            Serial.println("SELECT");
            result = 5;
            break;
        case 'e':
            Serial.println("ANSWER");
            result = 7;
            break;
        case 'q':
            Serial.println("BACK");
            result = 9;
            break;
        case '*':
            Serial.println(input);
            result = 19;
            break;
        case '#':
            Serial.println(input);
            result = 21;
            break;
        default:
            if (input >= '0' && input <= '9') {
                Serial.println(input);
                result += 10;
                result -= '1';
            }
            break;
        }
    }
    switch (result) {
    case 2:
        return UP;
    case 4:
        return LEFT;
    case 5:
        return SELECT;
    case 6:
        return RIGHT;
    case 7:
        return ANSWER;
    case 8:
        return DOWN;
    case 9:
        return DECLINE;
    case 19:
        return '*';

    case 20:
        return '0';

    case 21:
        return '#';

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