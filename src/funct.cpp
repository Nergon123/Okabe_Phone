#include "funct.h"
// TaskHandle_t TaskHandleATCommand;
unsigned int getIndexOfCount(int count, String input, String str, unsigned int fromIndex) {
    for (int i = 0; i < count; i++) {
        fromIndex = input.indexOf(str, fromIndex + 1);
    }
    return fromIndex;
}
void printT_S(String str){
    
    Serial.println(str);
    #ifdef LOG
    tft.println(str);
    #endif
}
String HEXTOASCII(String hex) {
    hex.toUpperCase();
    String output;

    for (int i = 0; i < hex.length(); i += 2) {
        char h  = hex[i];
        char l  = hex[i + 1];
        int  hv = (h >= '0' && h <= '9') ? h - '0' : h - 'A' + 10;
        int  lv = (l >= '0' && l <= '9') ? l - '0' : l - 'A' + 10;
        char c  = (hv << 4) | lv;
        output += c;
    }
    return output;
}
String sendATCommand(String command, uint32_t timeout, bool background) {
    bool _simIsBusy = simIsBusy;
    while (_simIsBusy) {
        delay(50);
        _simIsBusy = simIsBusy;
    }
    simIsBusy = true;
    Serial1.println(command); // Send the AT command

    String   response  = "";
    uint32_t startTime = millis();

    // Wait for response or timeout
    while (millis() - startTime < timeout) {
        while (Serial1.available()) {

            char c = Serial1.read(); // Read a single character
            response += c;           // Append it to the response
        }
    }
    if (response.indexOf("+CLIP:") != -1) {
        int indexClip = response.indexOf("+CLIP:");
        int comma     = getIndexOfCount(2, response, "\"", indexClip);
        Serial.println("STATUS:" + response.substring(comma, response.indexOf(',', comma + 1)));
    }
    if (!isCalling) {
        bool _isCalling = response.indexOf("RING\r") != -1;
        if (_isCalling) {
            int indexClip = response.indexOf("+CLIP:");
            int endindex  = response.indexOf("\r", indexClip);
            int firIndex  = response.indexOf("\"", indexClip);
            currentNumber = response.substring(firIndex, response.indexOf("\"", firIndex + 1));
            currentNumber.replace("\"", "");
            Serial.println(currentNumber);
        }
        isCalling = _isCalling;
    } else {
        if (response.indexOf("NO CARRIER") != -1) {
            Serial.println("Call Ended.");
            isCalling     = false;
            currentNumber = "";
        }
    }
    if (response.indexOf("ERROR") != -1) {
        Serial.println("AT COMMAND FAILED :" + command);

        Serial.println("===========FAILED===========\nRESPONSE:\n" +
                       response +
                       "\n============================\n");
    }

    simIsBusy = false;
    return response;
}

String getATvalue(String command, bool background = false) {

    String response = sendATCommand(command, 1000, background);
    Serial.println(response);
    String result = "";

    if (response.indexOf("ERROR") != -1) {
        return "ERROR";
    }

    int startIdx = response.indexOf(":");
    if (startIdx != -1) {

        int endIdx = response.indexOf("\r", startIdx);

        result = response.substring(startIdx + 1, endIdx);
        result.trim();
        Serial.println(result);
    } else {
        result = response;
        result.trim();
    }

    return result;
}
int getChargeLevel() {

    int toIcon = (chrg.getBatteryLevel() / 25) - 1;

    toIcon = (toIcon < 0) ? 0 : (toIcon > 3 ? 3 : toIcon);

    return toIcon;
}
int getSignalLevel() {

    int    signal = -1;

    String a      = getATvalue("AT+CREG?", true);

    //Serial.println("GETSIGNALLEVEL_CREG:" + a);

    if (a.charAt(2) == '1' || a.charAt(2) == '5') {
        String b = getATvalue("AT+CSQ");
        if (b != "ERROR") {
            char buf[5];
            b.substring(0, b.indexOf(',')).toCharArray(buf, 5);
            int strength = atoi(buf);
            if (strength != 99)
                if (signal > 3 || signal < 0)
                    signal = strength / 8;
        }
    }
    return signal;
}

void populateContacts() {
    String response = sendATCommand("AT+CPBR=1,100"); // Query contacts from index 1 to 100

    Serial.println(response);
    // Process the response
    int startIndex = 0;
    int endIndex   = 0;
    contactCount   = 0;

    while ((startIndex = response.indexOf("+CPBR: ", endIndex)) != -1) {
        startIndex += 7; // Skip "+CPBR: "
        endIndex     = response.indexOf('\n', startIndex);
        String entry = response.substring(startIndex, endIndex);

        // Split the entry into components
        int commaIndex = entry.indexOf(',');
        if (commaIndex == -1)
            break;

        // Extract index
        String indexStr     = entry.substring(0, commaIndex);
        int    contactIndex = indexStr.toInt();
        entry               = entry.substring(commaIndex + 1);

        commaIndex = entry.indexOf(',');
        if (commaIndex == -1)
            break;

        // Extract phone number
        String number = entry.substring(0, commaIndex);
        number.replace("\"", "");
        entry = entry.substring(commaIndex + 1);

        // Extract name
        String name = entry;
        name.replace("\"", "");
        name.replace("145,", "");
        name.replace("129,", "");

        // Populate the contact structure
        contacts[contactCount].index = contactIndex; // Use the index from +CPBR
        contacts[contactCount].phone = number;
        contacts[contactCount].name  = name;
        contactCount++;
        if (lastContactIndex < contactIndex) {
            lastContactIndex = contactIndex;
        }
        if (contactCount >= MAX_CONTACTS)
            break; // Prevent overflow
    }
}

bool checkButton(int pin) {
    if (digitalRead(pin) == LOW) {
        delay(50);
        while (digitalRead(pin) == LOW)
            ;
        return true;
    }
    return false;
}

int checkEXbutton() {

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

int GetState() {
    String result = sendATCommand("AT+CLCC");
    Serial.println("AAA" + result);
    if (result.indexOf("+CLCC") == -1 && result.indexOf("OK") != -1)
        return -1;
    int indexState = getIndexOfCount(2, result, ",", result.indexOf("+CLCC"));
    result         = result.substring(indexState, result.indexOf(",", indexState + 1));
    result.replace(",", "");
    result.trim();
    Serial.println(result);
    return atoi(result.c_str());
}

int buttonsHelding() {

    /*
     * SIDE BUTTON 10
     *  7 8 9
     * |M|I|B|
     *      ^=W
     * <=A  0=SP >=D
     * A=Q  V=S  D=E
     * 1   2   3
     * 4   5   6
     * 7   8   9
     * *   0   #
     *
     * M-MESSAGES
     * I-INFORMATION(????)
     * B-CONTACTS
     * C-CAMERA(???)
     * A-ANSWER
     * D-DECLINE
     * 0-SELECT
     *
     * because we don't have BACK button Button D also acts as BACK
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
    int result = checkEXbutton();
    if (result != 0)
        while (result == checkEXbutton())
            ;
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


    if (Serial.available()) {
        char input = Serial.read();
        switch (input) {
        case 'a':
            Serial.println("LEFT");
            return LEFT;
            break;
        case 'd':
            Serial.println("RIGHT");
            return RIGHT;
            break;
        case 'w':
            Serial.println("UP");
            return UP;
            break;
        case 's':
            Serial.println("DOWN");
            return DOWN;
            break;
        case ' ':
            Serial.println("SELECT");
            return SELECT;
            break;
        case 'e':
            Serial.println("ANSWER");
            return ANSWER;
            break;
        case 'q':
            Serial.println("BACK");
            return BACK;
            break;
        case '`':
            Serial.println("SOFTWARE RESET");
            ESP.restart();
            return -1;
            break;
        case 'm':
            Serial.println("MESSAGES");
            drawStatusBar();
            messages();
            MainScreen();
            return -1;
            break;
        case 'b':
            Serial.println("CONTACTS");
            drawStatusBar();
            contactss();
            MainScreen();
            return -1;
            break;
        case '*':
            Serial.println(input);
            return input;
            break;
        case '#':
            Serial.println(input);
            return input;
            break;
        default:
            if (input >= '0' && input <= '9') {
                Serial.println(input);
                return input;
            }
            break;
        }
    }

    return -1;
}


void parseMessages(Message *&msgs, int &count) {
    String response     = sendATCommand("AT+CMGL=\"ALL\",1");
    int    lastIndexOf  = 0;
    int    messageCount = 0;

    // Count the number of messages
    while ((lastIndexOf = response.indexOf("+CMGL:", lastIndexOf)) != -1) {
        messageCount++;
        lastIndexOf += 6; // Move past "+CMGL:"
    }

    if (messageCount > 0) {
        msgs        = new Message[messageCount];
        lastIndexOf = 0;

        for (int i = 0; i < messageCount; i++) {
            lastIndexOf = response.indexOf("+CMGL: ", lastIndexOf);
            if (lastIndexOf == -1)
                break;

            int firstComma  = response.indexOf(",", lastIndexOf);
            int secondComma = response.indexOf(",", firstComma + 1);
            int thirdComma  = response.indexOf(",", secondComma + 1);
            int fourthComma = response.indexOf(",", thirdComma + 1);

            // Extract index
            msgs[i].index = response.substring(lastIndexOf + 7, firstComma).toInt();

            // Extract status
            String statusStr = response.substring(response.indexOf("\"", firstComma) + 1, response.indexOf("\"", secondComma));
            if (statusStr.indexOf("UNREAD") != -1)
                msgs[i].status = status::NEW;
            else if (statusStr.indexOf("READ") != -1)
                msgs[i].status = status::READED;
            else
                msgs[i].status = status::NEW; // Default fallback

            // Extract sender number
            String number = response.substring(response.indexOf("\"", secondComma) + 1, response.indexOf("\"", response.indexOf("\"", secondComma) + 1));

            // Extract date
            int dateStart = response.indexOf("/", lastIndexOf);
            msgs[i].date  = response.substring(dateStart + 1, response.indexOf(",", dateStart));


            String checknumber;
            if (number.indexOf('+') != -1) {
                checknumber = number.substring(number.indexOf('+') + 3);
            } else {
                checknumber = number;
            }
            Contact curContact;
            curContact.phone = number;
            if (number.indexOf('p') != -1)
                curContact.name = "SERVICE NUMBER";
            else
                curContact.name = number;
            curContact.index = -1;
            for (int i = 0; i < contactCount; i++) {
                if (contacts[i].phone.indexOf(checknumber) != -1) {
                    curContact = contacts[i];
                    break;
                }
            }
            Serial.println(checknumber);
            msgs[i].contact = curContact;
            Serial.println("INDEX: " + String(msgs[i].index));
            Serial.println("STATUS: " + String((int)msgs[i].status)); // Cast enum to int
            Serial.println("DATE: " + msgs[i].date);
            Serial.println("NUMBER: " + number);
            Serial.println("CONTACT:" + msgs[i].contact.name);
            lastIndexOf += 7; // Move past "+CMGL:"
        }
    }
    count = messageCount;
}

int measureStringHeight(const String &text) {
    // STOLEN FROM CHATGPT
    int lines      = 1;
    int lineWidth  = 0;
    int spaceWidth = tft.textWidth(" ");
    int lineHeight = tft.fontHeight();

    String word = "";

    for (int i = 0; i < text.length(); i++) {
        char c = text[i];

        if (c == '\r' && i + 1 < text.length() && text[i + 1] == '\n') {
            lines++;
            lineWidth = 0;
            i++;
            continue;
        }

        if (c == '\n' || c == '\r') {
            lines++;
            lineWidth = 0;
            continue;
        }

        if (c == ' ' || i == text.length() - 1) {
            if (i == text.length() - 1 && c != ' ') {
                word += c;
            }

            int wordWidth = tft.textWidth(word);

            if (lineWidth + wordWidth > tft.width()) {
                lines++;
                lineWidth = wordWidth + spaceWidth;
            } else {
                lineWidth += wordWidth + spaceWidth;
            }

            word = "";
        } else {
            word += c;
        }
    }

    return lines * lineHeight;
}

void checkVoiceCall() {
    if (isCalling && !ongoingCall) {

        Contact calling;
        calling.phone = currentNumber;
        for (int i = 0; i < contactCount; i++) {
            if (contacts[i].phone.indexOf(currentNumber) != -1) {
                calling = contacts[i];
                break;
            }
        }
        ongoingCall = true;
        incomingCall(calling);
    }
}

bool _checkSim() {
    lastSIMerror = sendATCommand("AT+CPIN?");
    if (lastSIMerror.indexOf("READY") == -1) {
        lastSIMerror.trim();
        lastSIMerror = lastSIMerror.substring(lastSIMerror.indexOf("+CME") + 11);
        return false;
    } else
        return true;
}
bool checkSim() {
    if (!simIsUsable)
        ErrorWindow(lastSIMerror);
    return simIsUsable;
}
void playAudio(String path) {

    while (true /*STATEMENT PLACEHOLDER*/) {
        break;
    }
}