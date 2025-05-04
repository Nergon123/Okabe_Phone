#include "Serial.h"


// Highly experimental
int RunAction(String request) {
    String actions[] = {"RECEIVE", "SEND", "LIST", "DELETE", "WRITE", "READ", "EXIT"};
    int    action    = -1;

    for (int i = 0; i < ArraySize(actions); i++) {
        if (request.indexOf(actions[i]) != -1) {
            request.replace(actions[i], "");
            request.trim();
            action = i;
            break;
        }
    }

    if (action == -1) {
        return -1; // Invalid action
    }

    switch (action) {
    case 0: { // RECIEVE
        String fileName = request;
        if (fileName.length() == 0) {
            tft.println("Invalid file name.");
            return -1;
        }

        tft.fillScreen(TFT_BLACK);
        tft.setCursor(10, 30);
        tft.println("Receiving: " + fileName);
        Serial.println("START");

        if (SD.exists("/" + fileName)) {
            SD.remove("/" + fileName);
        }

        File file = SD.open("/" + fileName, FILE_WRITE);
        if (!file) {
            tft.println("Failed to open file.");
            Serial.println("Failed to open file.");
            return -1;
        }

        const int     BUFFER_SIZE = 512;
        uint8_t       buffer[BUFFER_SIZE];
        int           bytesReceived   = 0;
        unsigned long lastReceiveTime = millis();

        while (true) {
            int availableBytes = Serial.available();
            if (availableBytes > 0) {
                int toRead = Serial.readBytes(buffer, BUFFER_SIZE);
                file.write(buffer, toRead);
                bytesReceived += toRead;
                lastReceiveTime = millis();

                file.flush();

                Serial.println("ACK");
            }

            if (millis() - lastReceiveTime > 10000) {
                break;
            }
        }

        file.flush();
        file.close();
        Serial.println("DONE");

        tft.setCursor(10, 100);
        tft.println("Transfer Complete!");
        break;
    }
    case 1: { // SEND
        String fileName = request;
        if (!SD.exists("/" + fileName)) {
            tft.println("ERROR: File not found.");
            return -1;
        }

        File file = SD.open("/" + fileName, FILE_READ);
        if (!file) {
            tft.println("ERROR: Cannot open file.");
            return -1;
        }

        Serial.println("START");

        const int BUFFER_SIZE = 512;
        uint8_t   buffer[BUFFER_SIZE];

        while (file.available()) {
            int bytesRead = file.read(buffer, BUFFER_SIZE);
            Serial.write(buffer, bytesRead);

            String ack;
            while (ack != "ACK")
                if (Serial.available())
                    ack = Serial.readStringUntil('\n');
        }

        file.close();
        ulong baud = Serial.baudRate();
        Serial.end();
        delay(100);
        Serial.begin(baud);

        Serial.println("DONE");
        break;
    }

    case 2: { // LIST FILES
        File root = SD.open(request);
        if (!root) {
            Serial.println("ERROR: Cannot open directory.");
            return -1;
        }

        Serial.println("**FILES:");
        while (true) {
            File entry = root.openNextFile();
            if (!entry) {
                break;
            }
            if (entry.isDirectory()) {
                // For directories, append a "/" and set size to 0
                Serial.println(String(entry.name()) + "/,0");
            } else {
                // For files, include the file size
                Serial.println(String(entry.name()) + "," + entry.size());
            }
            entry.close();
        }
        Serial.println("**END");
        break;
    }

    case 3: { // DELETE FILE
        String fileName = request;
        if (SD.exists("/" + fileName)) {
            SD.remove("/" + fileName);
            Serial.println("DELETED");
        } else {
            Serial.println("ERROR: File not found.");
        }
        break;
    }
    case 4: { // WRITE
        int firstSpace  = request.indexOf(' ');
        int secondSpace = request.indexOf(' ', firstSpace + 1);

        String filePath = request.substring(0, firstSpace);
        int    offset   = request.substring(firstSpace + 1, secondSpace).toInt();
        int    length   = request.substring(secondSpace + 1).toInt();

        File file = SD.open(filePath, FILE_WRITE);
        if (!file) {
            Serial.println("ERROR: Cannot open file.");
            return -1;
        }

        if (!file.seek(offset)) {
            Serial.println("ERROR: Seek failed.");
            file.close();
            return -1;
        }

        uint8_t buffer[512];
        int     bytesRead = 0;
        while (bytesRead < length) {
            int toRead = min(length - bytesRead, (int)sizeof(buffer));
            int read   = Serial.readBytes(buffer, toRead);
            file.write(buffer, read);
            bytesRead += read;
            file.flush();
            Serial.println("ACK");
        }

        file.flush();
        file.close();
        Serial.println("DONE");
        break;
    }
    case 5: { // READ
        int firstSpace  = request.indexOf(' ');
        int secondSpace = request.indexOf(' ', firstSpace + 1);

        String filePath = request.substring(0, firstSpace);
        int    offset   = request.substring(firstSpace + 1, secondSpace).toInt();
        int    length   = request.substring(secondSpace + 1).toInt();

        File file = SD.open(filePath, FILE_READ);
        if (!file) {
            Serial.println("ERROR: Cannot open file.");
            return -1;
        }

        if (!file.seek(offset)) {
            Serial.println("ERROR: Seek failed.");
            file.close();
            return -1;
        }

        uint8_t buffer[512];
        int     bytesRead = 0;
        while (bytesRead < length) {
            int    toRead = min(length - bytesRead, (int)sizeof(buffer));
            size_t read   = file.read(buffer, toRead);
            Serial.write(buffer, read);
            bytesRead += read;

            String ack;
            while (ack != "ACK")
                if (Serial.available())
                    ack = Serial.readStringUntil('\n');
        }
        file.close();
        Serial.println("DONE");
        break;
        break;
    }
    case 6: // EXIT
        return 255;
        break;
    }

    return action;
}

// Highly experimental
void SerialConnection() {
    suspendCore(true);
    fastMode(true);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, 30);
    Serial.updateBaudRate(FAST_SERIAL_BAUD_RATE);
    tft.println("WAITING FOR CONNECTION...");
    bool exit = false;
    while (!exit) {
        // Wait

        while (!Serial.available() & !exit) {
            ulong ltime = millis();
            while ((millis() - ltime < 1000) && !exit) {
                exit = buttonsHelding(false) == BACK;
            }
            Serial.println("READY");
        }
        String action = Serial.readStringUntil('\n');
        action.trim();
        if (RunAction(action) == 255)
            break;
    }

    Serial.updateBaudRate(FAST_SERIAL_BAUD_RATE);
    fastMode(false);
    suspendCore(false);
}
