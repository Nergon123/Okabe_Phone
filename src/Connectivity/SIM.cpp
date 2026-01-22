#include "SIM.h"
#ifndef PC
#define SIM_BAUD_RATE 115200
static const char* TAG = "SMS_PDU";
static uint8_t     hexNibble(char c) {
    if (c >= '0' && c <= '9') { return c - '0'; }
    if (c >= 'A' && c <= 'F') { return c - 'A' + 10; }
    if (c >= 'a' && c <= 'f') { return c - 'a' + 10; }
    return 0;
}
static uint8_t hexByte(const char* s) { return (hexNibble(s[0]) << 4) | hexNibble(s[1]); }
static NString swapSemi(const char* s, int len) {
    NString out;
    for (int i = 0; i < len; i += 2) {
        out += s[i + 1];
        if (s[i] != 'F') { out += s[i]; }
    }
    return out;
} // GSM 7-bit unpack
static NString decode7bit(const uint8_t* data, int septets, int skipBits) {
    NString  out;
    uint16_t carry     = 0;
    int      carryBits = skipBits;
    for (int i = 0; i < septets; i++) {
        int      byteIndex = (i * 7 + skipBits) / 8;
        int      bitOffset = (i * 7 + skipBits) % 8;
        uint16_t v         = data[byteIndex] | (data[byteIndex + 1] << 8);
        char     c         = (v >> bitOffset) & 0x7F;
        if (c != 0) { out += c; }
    }
    return out;
}

NString formatTimestamp(const NString& scts) {
    if (scts.length() < 14) { return scts; }
    int  year   = 2000 + atoi(scts.substring(0, 2).c_str());
    int  month  = atoi(scts.substring(2, 4).c_str());
    int  day    = atoi(scts.substring(4, 6).c_str());
    int  hour   = atoi(scts.substring(6, 8).c_str());
    int  minute = atoi(scts.substring(8, 10).c_str());
    int  second = atoi(scts.substring(10, 12).c_str());
    char buf[32];
    snprintf(buf, 32, "%04d/%02d/%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
    return NString(buf);
}

NString formatShortDate(const NString& scts) {
    if (scts.length() < 14) { return scts; }
    int  month = atoi(scts.substring(2, 4).c_str());
    int  day   = atoi(scts.substring(4, 6).c_str());
    char buf[8];
    snprintf(buf, 8, "%02d/%02d", month, day);
    return NString(buf);
}

std::vector<Message> parseMessages() {
    std::vector<Message> msgs;

    NString response = sendATCommand("AT+CMGL=4"); // PDU mode, all messages

    // Split response into lines
    std::vector<NString> lines;
    int                  s = 0;
    while (true) {
        int e = response.indexOf('\n', s);
        if (e == -1) {
            lines.push_back(response.substring(s));
            break;
        }
        lines.push_back(response.substring(s, e));
        s = e + 1;
    }

    for (size_t i = 0; i + 1 < lines.size(); i++) {

        if (!lines[i].startsWith("+CMGL:")) { continue; }

        Message msg;
        ESP_LOGI(TAG, "---- NEW MESSAGE ----");

        int c1     = lines[i].indexOf(',');
        msg.index  = lines[i].substring(7, c1).toInt();
        msg.status = lines[i].substring(c1 + 1, lines[i].indexOf(',', c1 + 1)).toInt();

        const char* p = lines[i + 1].c_str();
        ESP_LOGI(TAG, "PDU: %s", p);

        // --- SMSC ---
        int smscLen = hexByte(p);
        p += (1 + smscLen) * 2;

        // --- First octet ---
        uint8_t fo   = hexByte(p);
        bool    udhi = fo & 0x40;
        p += 2;
        ESP_LOGD(TAG, "FO=0x%02X UDHI=%d", fo, udhi);

        // --- Originating address ---
        int oaLen = hexByte(p);
        p += 2;
        uint8_t oaType = hexByte(p);
        p += 2;
        int oaBytes = (oaLen + 1) / 2;

        // Handle alphanumeric senders
        if ((oaType & 0x70) == 0x50) { // alphanumeric
            uint8_t buf[30] = {0};
            for (int b = 0; b < oaBytes; b++) { buf[b] = hexByte(p + b * 2); }
            msg.contact.phone = decode7bit(buf, oaLen, 0);
        }
        else { msg.contact.phone = swapSemi(p, oaBytes * 2); }
        p += oaBytes * 2;
        bool foundContact = false;
        for (Contact contact : contacts) {
            if (msg.contact.phone.length() <= 4) { continue; }
            if (contact.phone.indexOf(msg.contact.phone.substring(4)) != -1) {
                msg.contact  = contact;
                foundContact = true;
            }
        }
        if (!foundContact) { msg.contact.name = msg.contact.phone; }
        ESP_LOGI(TAG, "FROM: %s", msg.contact.phone.c_str());

        // PID + DCS
        p += 2;
        uint8_t dcs = hexByte(p);
        p += 2;
        ESP_LOGD(TAG, "DCS=0x%02X", dcs);

        // --- Timestamp ---
        NString timestamp = swapSemi(p, 14);
        msg.longdate      = formatTimestamp(timestamp);
        msg.date          = formatShortDate(timestamp);
        p += 14;
        ESP_LOGI(TAG, "TIME: %s", msg.longdate.c_str());
        ESP_LOGI(TAG, "DATE: %s", msg.date.c_str());

        // --- User Data ---
        int udl = hexByte(p);
        p += 2;
        int     skipBits  = 0;
        int     udhLen    = 0;
        uint8_t concatRef = 0, part = 1, total = 1;

        if (udhi) {
            udhLen = hexByte(p);
            if (hexByte(p + 2) == 0x00) { // concatenation IEI
                concatRef = hexByte(p + 6);
                total     = hexByte(p + 8);
                part      = hexByte(p + 10);
                ESP_LOGI(TAG, "CONCAT ref=%d part=%d/%d", concatRef, part, total);
                msg.part  = part;
                msg.total = total;
            }
            p += (udhLen + 1) * 2;
            skipBits = ((udhLen + 1) * 8) % 7;
            if (skipBits) { skipBits = 7 - skipBits; }
        }

        int     dataBytes = (dcs & 0x0C) == 0x08 ? udl : (udl * 7 + 7) / 8;
        uint8_t buf[180]  = {0};
        for (int b = 0; b < dataBytes; b++) { buf[b] = hexByte(p + b * 2); }

        if ((dcs & 0x0C) == 0x08) {
            // UCS2 → UTF-8
            NString out;
            for (int j = 0; j + 1 < dataBytes; j += 2) {
                uint16_t ch = (buf[j] << 8) | buf[j + 1];
                if (ch < 0x80) { out += (char)ch; }
                else if (ch < 0x800) {
                    out += (char)(0xC0 | (ch >> 6));
                    out += (char)(0x80 | (ch & 0x3F));
                }
                else {
                    out += (char)(0xE0 | (ch >> 12));
                    out += (char)(0x80 | ((ch >> 6) & 0x3F));
                    out += (char)(0x80 | (ch & 0x3F));
                }
            }
            msg.content = out;
        }
        else { msg.content = decode7bit(buf, udl, skipBits); }

        ESP_LOGI(TAG, "TEXT: %s", msg.content.c_str());
        msgs.push_back(msg);
        i++; // skip PDU line
    }
    return msgs;
}

/*
 * Send AT command to Sim Card module
 * @param command AT command to send
 * @param timeout Timeout for the command in milliseconds
 * @param background If true, then command is not important
 * @return Response from the SIM card module
 */
NString sendATCommand(NString command, uint32_t timeout, bool background) {
    if (SimSerial.baudRate() != SIM_BAUD_RATE) { SimSerial.updateBaudRate(SIM_BAUD_RATE); }
    bool  _simIsBusy = simIsBusy;
    ulong timer      = millis();
    while (_simIsBusy) {
        if (millis() - timer > 10000) { break; }
        hw->delay(50);
        _simIsBusy = simIsBusy;
    }
    simIsBusy = true;
    SimSerial.println(command.c_str()); // Send the AT command

    NString  response  = "";
    uint32_t startTime = hw->millis();

    // Wait for response or timeout
    while (hw->millis() - startTime < timeout) {
        while (SimSerial.available()) {
            char c = SimSerial.read(); // Read a single character
            response += c;             // Append it to the response
        }
    }

    if (response.indexOf("+CLIP:") != -1) {
        int indexClip = response.indexOf("+CLIP:");
        int comma     = getIndexOfCount(2, response, "\"", indexClip);
        ESP_LOGI("INFO", "STATUS: %s",
                 response.substring(comma, response.indexOf(',', comma + 1)).c_str());
    }

    if (!isCalling) {
        isCalling = response.indexOf("RING\r") != -1;
        if (isCalling) {
            int indexClip = response.indexOf("+CLIP:");
            int firIndex  = response.indexOf("\"", indexClip);
            currentNumber = response.substring(firIndex, response.indexOf("\"", firIndex + 1));
            currentNumber.replace("\"", "");
            ESP_LOGI("SIM", "Calling number: %s", currentNumber.c_str());
        }
    }
    else {
        if (response.indexOf("NO CARRIER") != -1) {
            ESP_LOGI("SIM", "Call Ended.");
            isCalling     = false;
            currentNumber = "";
        }
    }

    if (response.indexOf("ERROR") != -1) {
        ESP_LOGE("SIM", "ERR FOR %s : %s", command.c_str(), response.c_str());
    }

    simIsBusy = false;
    return response;
}

/* Get string value after first colon
 * @param command AT command to send
 * @param background If true, then command is not important
 * @return Response from the SIM card module
 */
NString getATvalue(NString command, bool background) {

    NString response = sendATCommand(command, 1000, background);
    NString result   = "";

    if (response.indexOf("ERROR") != -1) { return "ERROR"; }

    int startIdx = response.indexOf(":");
    if (startIdx != -1) {

        int endIdx = response.indexOf("\r", startIdx);

        result = response.substring(startIdx + 1, endIdx);
        result.trim();
    }
    else {
        result = response;
        result.trim();
    }

    return result;
}

/*
 * Check if SIM card is usable
 * @return true if SIM card is usable, false otherwise
 */
bool _checkSim() {
    lastSIMerror = sendATCommand("AT+CPIN?");
    if (lastSIMerror.indexOf("READY") == -1) {
        lastSIMerror.trim();
        lastSIMerror = lastSIMerror.substring(lastSIMerror.indexOf("+CME") + 11);
        return false;
    }
    else { return true; }
}

void simInterrupt() { ESP_LOGI("SIM", "INTERRUPT"); }
// Check if someone calling (Function subject to change. I need to use interrupts for that)
void checkVoiceCall() {
    if (isCalling && !ongoingCall) {

        Contact calling;
        calling.phone = currentNumber;
        for (Contact _cont : contacts) {
            if (_cont.phone.indexOf(currentNumber) != -1) {
                calling = _cont;
                break;
            }
        }
        ongoingCall = true;
        incomingCall(calling);
    }
}

// Throw full screen error if there no sim card
bool checkSim() {
    if (!simIsUsable) { InfoWindow(lastSIMerror); }
    return simIsUsable;
}

/*
 * Get Call status from SIM Card Module
 *
 * Status defined (see Defines.h):
 * `0` ACTIVE
 * `1` HELD
 * `2` DIALING
 * `3` ALERTING
 * `4` INCOMING
 * `5` WAITING
 * `6` DISCONNECT
 */
int GetState() {
    NString result = sendATCommand("AT+CLCC");
    ESP_LOGI("GET CALL STATE", "AT+CLCC Result:%s", result.c_str());
    if (result.indexOf("+CLCC") == -1 && result.indexOf("OK") != -1) { return stateCall; }
    int indexState = getIndexOfCount(2, result, ",", result.indexOf("+CLCC"));
    result         = result.substring(indexState, result.indexOf(",", indexState + 1));
    result.replace(",", "");
    result.trim();

    return atoi(result.c_str());
}

// Function to initialize the SIM card
// This function sends AT commands to the SIM card to set it up
void initSim() {
    ESP_LOGI("BOOT/SIM", "%s",
             sendATCommand("AT+CMEE=2").c_str()); // Enable verbose error reporting
    ESP_LOGI("BOOT/SIM", "%s",
             sendATCommand("AT+CLIP=1").c_str()); // Enable caller ID reporting
    // Report a list of current calls of ME automatically
    // when the current call status changes.
    ESP_LOGI("BOOT/SIM", "%s", sendATCommand("AT+CLCC=1").c_str());
    ESP_LOGI("BOOT/SIM", "%s",
             sendATCommand("AT+CSCS=\"GSM\"").c_str()); // Set character set to GSM
    ESP_LOGI("BOOT/SIM", "%s",
             sendATCommand("AT+CMGF=0").c_str()); // Set SMS mode to PDU
    simIsUsable = _checkSim();                    // Check if SIM card is usable
}

// Enable AT commands console
void AT_test() {
    tft.fillScreen(0);
    changeFont(0);
    tft.setCursor(0, 0);
    tft.println("AT COMMANDS CONSOLE\n\nWaiting for connection...\n\nIf you got here by mistake "
                "\ntry to press RESET \nand make sure # button didn't stuck...");
    while (true) {
        if (Serial.available()) {
            tft.fillScreen(0);
            tft.setCursor(0, 0);
            tft.setTextSize(2);

            tft.setTextColor(0xF800);
            tft.println("\nAT COMMANDS CONSOLE\n (TO EXIT TYPE :q)\n");
            tft.setTextColor(0xFFFF);
            tft.setTextSize(1);
            NString req;

            tft.println("REQUEST:");

            char t = Serial.read();
            req += t;
            tft.print(t);
            while (t != '\n') {
                if (Serial.available()) {
                    t = Serial.read();
                    req += t;
                    tft.print(t);
                }
            }
            req.replace('\n', '\0');

            if (req.indexOf(":q") != -1) { break; }
            NString ans = sendATCommand(req);

            tft.println("\nANSWER: " + ans);
            Serial.println(ans.c_str());
        }
    }
}
// Get cellular signal level (from 0 as "unavaliable" to 4)
int getSignalLevel() {

    int signal = -1;

    NString a = getATvalue("AT+CREG?", true);

    // Serial.println("GETSIGNALLEVEL_CREG:" + a);

    if (a.charAt(2) == '1' || a.charAt(2) == '5') {
        NString b = getATvalue("AT+CSQ");

        if (b != "ERROR") {

            char buf[5];
            b.substring(0, b.indexOf(',')).toCharArray(buf, 5);
            int strength = atoi(buf);

            if (strength != 99) { signal = strength / 8; }
        }
    }
    return signal + 1;
}

// Populate contact list
void populateContacts() {
    NString response = sendATCommand("AT+CPBR=1,100"); // Query contacts from index 1 to 100

    Serial.println(response.c_str());
    // Process the response
    int startIndex = 0;
    int endIndex   = 0;

    while ((startIndex = response.indexOf("+CPBR: ", endIndex)) != -1) {
        startIndex += 7; // Skip "+CPBR: "
        endIndex      = response.indexOf('\n', startIndex);
        NString entry = response.substring(startIndex, endIndex);

        // Split the entry into components
        int commaIndex = entry.indexOf(',');
        if (commaIndex == -1) { break; }

        // Extract index
        NString indexStr     = entry.substring(0, commaIndex);
        int     contactIndex = indexStr.toInt();
        entry                = entry.substring(commaIndex + 1);

        commaIndex = entry.indexOf(',');
        if (commaIndex == -1) { break; }

        // Extract phone number
        NString number = entry.substring(0, commaIndex);
        number.replace("\"", "");
        entry = entry.substring(commaIndex + 1);

        // Extract name
        NString name = entry;
        name.replace("\"", "");
        name.replace("145,", "");
        name.replace("129,", "");

        // Populate the contact structure
        Contact tempContact;
        tempContact.index = contactIndex; // Use the index from +CPBR
        tempContact.phone = number;
        tempContact.name  = name;
        contacts.push_back(tempContact);
    }
}
#else

std::vector<Message> parseMessages(){
    return std::vector<Message>{};
}
void    AT_test() {};
NString sendATCommand(NString command, uint32_t timeout, bool background) {
    (void)command;
    (void)timeout;
    (void)background;
    return "PC";
};
NString getATvalue(NString command, bool background) {
    (void)command;
    (void)background;
    return "PC";
};
bool checkSim() { return true; };
bool _checkSim() { return true; };
void initSim() {};
int  getSignalLevel() { return 0; };
void populateContacts() {};
void checkVoiceCall() {};
int  GetState() { return 0; };
#endif