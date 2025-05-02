#include "SIM.h"

// Send AT command to Sim Card module
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
            // int endindex  = response.indexOf("\r", indexClip);
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

// Get string value after first colon
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
// Check if sim card is there
bool _checkSim() {
    lastSIMerror = sendATCommand("AT+CPIN?");
    if (lastSIMerror.indexOf("READY") == -1) {
        lastSIMerror.trim();
        lastSIMerror = lastSIMerror.substring(lastSIMerror.indexOf("+CME") + 11);
        return false;
    } else
        return true;
}

// Check if someone calling (Function subject to change. I need to use interrupts for that)
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


// Throw full screen error if there no sim card
bool checkSim() {
    if (!simIsUsable)
        ErrorWindow(lastSIMerror);
    return simIsUsable;
}


// Get Call status from SIM Card Module
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

// Function to initialize the SIM card
// This function sends AT commands to the SIM card to set it up
void initSim() {
    printT_S(sendATCommand("AT+CMEE=2"));
    printT_S(sendATCommand("AT+CLIP=1"));
    printT_S(sendATCommand("AT+CLCC=1"));
    printT_S(sendATCommand("AT+CSCS=\"GSM\""));
    printT_S(sendATCommand("AT+CMGF=1"));
    simIsUsable = _checkSim();
}

void AT_test() {
    tft.fillScreen(0);
    changeFont(0);
    tft.setCursor(0, 0);
    tft.println("Waiting for connection...");
    while (true) {
        if (Serial.available()) {
            tft.fillScreen(0);
            tft.setCursor(0, 0);
            tft.setTextSize(2);

            tft.setTextColor(0xF800);
            tft.println("\nAT COMMANDS CONSOLE\n (TO EXIT TYPE :q)\n");
            tft.setTextColor(0xFFFF);
            tft.setTextSize(1);
            String req;

            tft.println("REQUEST:");
            req = Serial.readString();
            tft.print(req + "\n");
            if (req.indexOf(":q") != -1) {
                break;
            }
            String ans = sendATCommand(req);

            tft.println("\nANSWER: " + ans);
            Serial.println(ans);
        }
    }
}
// Get cellular signal level (from -1 as "unavaliable" to 3)
int getSignalLevel() {

    int signal = -1;

    String a = getATvalue("AT+CREG?", true);

    // Serial.println("GETSIGNALLEVEL_CREG:" + a);

    if (a.charAt(2) == '1' || a.charAt(2) == '5') {
        String b = getATvalue("AT+CSQ");

        if (b != "ERROR") {

            char buf[5];
            b.substring(0, b.indexOf(',')).toCharArray(buf, 5);
            int strength = atoi(buf);

            if (strength != 99) {
                signal = strength / 8;
            }
        }
    }
    return signal + 1;
}

// Populate contact list
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
            break;
    }
}