#include "screens.h"

void callActivity(Contact contact);
void incomingCall(Contact contact);
void inbox();
void numberInput(char first);
bool messageActivity(Message message);
void messageActivityOut(Contact contact, String subject, String content, bool sms);
void ringtoneSelector(bool isMail);
void SerialConnection();
void WiFiList();

// Function to set the time
// This function is called when the user wants to set the time
void setTime(time_t *time) {
    drawFromSd(0, 51, BACKGROUND_IMAGE);
    drawFromSd(0, 26, BLUEBAR_IMAGE);
    drawFromSd(0, 26, BLUEBAR_ICONS[2]); // Bluebar icon
    changeFont(1);
    tft.setCursor(30, 44);
    tft.setTextColor(0xFFFF);
    tft.setTextSize(1);
    tft.print("Date/Time Settings");
    tft.setTextColor(0);
    tft.setCursor(90, 77);
    tft.print("DATE");
    tft.setCursor(57 + 28, 109);
    tft.print("/");
    tft.setCursor(93 + 28, 109);
    tft.print("/");
    tft.setCursor(90, 155);
    tft.print("TIME");
    tft.setCursor(111, 187);
    tft.print(":");

    tm  tm_time   = *gmtime(&systemTime);
    int temp_year = 1900 + tm_time.tm_year;

    int  choice    = 0;
    bool exit      = false;
    bool renderall = true;
    int  direction = LEFT;
    while (!exit) {

        Serial.println(choice);

        sNumberChange(57, 90, 25, 25, tm_time.tm_mday, 1, 31, choice == 0 && !renderall, &direction);
        sNumberChange(93, 90, 25, 25, tm_time.tm_mon, 1, 12, choice == 1 && !renderall, &direction);
        sNumberChange(129, 90, 50, 25, temp_year, 1900, 2100, choice == 2 && !renderall, &direction);
        sNumberChange(83, 170, 25, 25, tm_time.tm_hour, 0, 23, choice == 3 && !renderall, &direction);
        sNumberChange(117, 170, 25, 25, tm_time.tm_min, 0, 59, choice == 4 && !renderall, &direction);
        bool confirm = button("CONFIRM", 10, 280, 100, 30, choice == 5 && !renderall, &direction);
        bool cancel  = button("CANCEL", 130, 280, 100, 30, choice == 6 && !renderall, &direction);

        Serial.println(direction);

        if (!renderall) {
            if (direction == RIGHT)
                choice++;
            else if (direction == LEFT)
                choice--;
            if (choice > 6)
                choice = 0;
            else if (choice < 0)
                choice = 6;
        }

        renderall = !renderall;

        if (confirm) {
            tm_time.tm_year = temp_year - 1900;
            Serial.println("Local time updated!");
            SaveTime(mktime(&tm_time));
            exit = true;
        }
        if (cancel) {
            exit = true;
        }
    }
}

// Function to show a confirmation window
// This function is called when the user wants to confirm an action
bool confirmation(String reason) {
    drawWallpaper();
    drawFromSd(0, 90, FULL_SCREEN_NOTIFICATION_IMAGE);
    changeFont(1);
    tft.setCursor(45, 110);
    tft.setTextColor(0);
    tft.println("CONFIRMATION");
    if (tft.textWidth(reason) < 240)
        tft.setCursor((230 - tft.textWidth(reason)) / 2, 130);
    else
        tft.setCursor(0, 130);
    tft.println(SplitString(reason));

    int  pos = 0;
    int  direction;
    bool exit = false;

    while (!exit) {
        if (button("YES", 120, 190, 80, 30, pos, &direction)) {
            return true;
        } else if (button("NO", 30, 190, 80, 30, !pos, &direction)) {
            return false;
        }
        if (direction > 1)
            pos = !pos;
    }
    return false;
}

// Function to show an error window
// This function is called when an error occurs
void ErrorWindow(String reason) {

    int xpos = 0;
    drawWallpaper();
    drawFromSd(0, 90, FULL_SCREEN_NOTIFICATION_IMAGE);
    tft.setTextSize(1);
    tft.setCursor(80, 120);
    changeFont(1);
    tft.setTextColor(0xF800);
    tft.println("ERROR");
    tft.setTextColor(0);
    if (tft.textWidth(reason) < 240) {
        xpos = (240 - tft.textWidth(reason)) / 2;
    }
    tft.setCursor(xpos, 150);
    tft.print(SplitString(reason));
    while (buttonsHelding() == -1)
        ;
}

// Function to show the messages menu
void messages() {

    drawFromSd(0, 26, SUBMENU_BACKGROUND);
    drawFromSd(0, 26, MAIL_HEADER_IMAGE);
    drawFromSd(0, 68, WHITE_BOTTOM_IMAGE);
    String entries[] = {"Inbox", "Outbox"};
    int    ch        = choiceMenu(entries, ArraySize(entries), false);
    switch (ch) {
    case 0:
        inbox(false);
        break;
    case 1:
        inbox(true);
        break;
    default:
        break;
    }
    currentScreen = SCREENS::MAINMENU;
}

// Function to show menu with additional features
void e() {

    int choice = -2;
    while (choice != -1) {
        String debug[] = {
            "Delete File",
            "Cat",
            "PC Connection",
            "Execute Application",
            "Set Date/Time",
            "Connect To Wi-Fi"};
        choice = listMenu(debug, ArraySize(debug), false, 2, "Additional Features");
        String path;
        File   file;
        switch (choice) {
        case 0:
            path = fileBrowser();
            SD.remove(path);
            break;
        case 1:
            path = fileBrowser();
            tft.fillScreen(0);
            tft.setCursor(0, 0);
            changeFont(0);
            tft.setTextColor(TFT_WHITE);
            file = SD.open(path);
            file.seek(0);
            if (file)
                while (file.available()) {
                    tft.print((char)file.read());
                }
            file.close();
            delay(1000);
            while (buttonsHelding() == -1)
                ;
            sBarChanged = true;
            drawStatusBar();
            break;
        case 2:
            SerialConnection();
            break;
        case 3:
            execute_application();
            break;
        case 4:
            setTime(&systemTime);
            break;
        case 5:
            WiFiList();
            break;
        }

        break;
    }
    currentScreen = SCREENS::MAINMENU;
}

// Function to show the settings menu
// This function is called when the user wants to change settings
// It allows the user to change the wallpaper, ringtones, etc.
void settings() {

    drawFromSd(0, 26, SUBMENU_BACKGROUND); // Menu background
    drawFromSd(0, 26, SETTINGS_HEADER_IMAGE);
    String lol[] = {
        "Change Wallpaper",
        "Phone ringtone",
        "Mail ringtone",
    };
    int a     = choiceMenu(lol, 3, false);
    int pic   = -1;
    int picch = -2;

    while (true) {
        switch (a) {
        case -1:
            currentScreen = SCREENS::MAINMENU;
            return;
            break;
        case 0:
            pic = gallery();

            if (pic == -1) {
                currentScreen = SCREENS::MAINMENU;
                return;
            }
            break;
        case 1:
            ringtoneSelector(false);
            return;
            break;
        case 2:
            ringtoneSelector(true);
            return;
            break;
        default:
            currentScreen = SCREENS::MAINMENU;
            return;
            break;
        }
        Serial.println(pic);
        String galch[] = {"Preview", "Confirm"};
        if (pic != -1 && pic != lastImage) {

            picch = choiceMenu(galch, 2, true);
            switch (picch) {
            case 0:
                drawFromSd(WALLPAPER_IMAGES_BASE.address + WALLPAPER_IMAGES_MULTIPLIER * pic, 0, 26, WALLPAPER_IMAGES_BASE.w, WALLPAPER_IMAGES_BASE.h, "/" + resPath, 0, 0);
                while (buttonsHelding() != BACK)
                    ;
                break;
            case 1:
                preferences.begin("settings", false);
                preferences.putUInt("wallpaperIndex", pic);
                preferences.putString("wallpaper", "/null");
                preferences.end();
                wallpaperIndex = pic;
                free(wallpaper);
                wallpaper            = nullptr;
                currentWallpaperPath = "/null";
                currentScreen        = SCREENS::MAINSCREEN;
                return;
                break;
            default:

                break;
            }
        } else if (pic == lastImage) {
            String path = fileBrowser(SD.open("/"), ".png");
            picch       = choiceMenu(galch, 2, true);
            switch (picch) {
            case 0:
                drawPNG(path.c_str());
                break;
            case 1:
                preferences.begin("settings", false);
                preferences.putUInt("wallpaperIndex", -1);
                preferences.putString("wallpaper", path);
                preferences.end();
                free(wallpaper);
                wallpaper            = nullptr;
                wallpaperIndex       = -1;
                currentWallpaperPath = path;
                currentScreen        = SCREENS::MAINSCREEN;
                return;
                break;
            default:

                break;
            }
        }
    }
    currentScreen = SCREENS::MAINMENU;
}

// Function to show the main menu
void MainMenu() {

    drawFromSd(0, 26, MENU_BACKGROUND);   // Menu background
    drawFromSd(51, 71, MENU_ON_ICONS[0]); // Menu icon

    int choice     = 0;
    int old_choice = 0;
    while (true) {
        switch (buttonsHelding()) {
        case BACK: {
            currentScreen = SCREENS::MAINSCREEN;
            return;
            break;
        }
        case SELECT: {
            switch (choice) {
            case 0:

                currentScreen = SCREENS::MESSAGES;
                return;
                break;
            case 1:
                currentScreen = SCREENS::CONTACTS;
                return;
                break;
            case 2:
                currentScreen = SCREENS::E;
                return;
                break;
            case 3:
                currentScreen = SCREENS::SETTINGS;
                return;
                break;
            }
            drawFromSd(0, 26, MENU_BACKGROUND);
            rendermenu(choice, choice);
            break;
        }

        case UP: {
            old_choice = choice;
            choice -= 2;
            rendermenu(choice, old_choice);
            break;
        }

        case DOWN: {
            old_choice = choice;
            choice += 2;
            rendermenu(choice, old_choice);
            break;
        }
        case LEFT: {
            old_choice = choice;
            choice--;
            rendermenu(choice, old_choice);
            break;
        }

        case RIGHT: {
            old_choice = choice;
            choice++;
            rendermenu(choice, old_choice);
            break;
        }
        }
    }
}

// Function to show the gallery menu
// This function is called when the user wants to change the wallpaper
int gallery() {
    if (!SD.exists(resPath))
        return lastImage;
    mOption wallnames[] = {
        {"Wallpaper 1"},
        {"Wallpaper 2"},
        {"Wallpaper 3"},
        {"IBN5100"},
        {"Red jelly"},
        {"The head of doll"},
        {"Mayuri jelly"},
        {"Fatty Gero Froggy"},
        {"Burning Gero Froggy"},
        {"Upa"},
        {"Metal Upa"},
        {"Seira"},
        {"Seira After awaking"},
        {"Gero Froggy"},
        {"Cat Gero Froggy"},
        {"Cow Gero Froggy"},
        {"FES"},
        {"Gero Froggies"},
        {"Calico Gero Froggies"},
        {"Gold Upa"},
        {"FES2"},
        {"Erin 1"},
        {"Erin 2"},
        {"Orgel Sisters"},
        {"Mayuri"},
        {"Kurisu"},
        {"Moeka"},
        {"Luka"},
        {"Faris"},
        {"Suzuha"},
        {"UNCONFIRMED"},
        {"Popping steiner"},
        {"Wallpaper 4"},
        {"NukariyaIce"},
        {"MayQueen"},
        {"Upa ♪"},
        {"Wallpaper 5"},
        {"Rabikuro"},
        {"Wallpaper 6"},
        {"Space Froggies"},
        {"Wallpaper 7"},
        {"Nae"},
        {"Pick wallpaper..."}};
    // drawWallpaper();
    for (int i = 0; i < ArraySize(wallnames) - 1; i++) {
        wallnames[i].icon = SDImage(WALLPAPER_ICONS_BASE.address + (WALLPAPER_ICONS_MULTIPLIER * i), WALLPAPER_ICONS_BASE.w, WALLPAPER_ICONS_BASE.h);
    }
    return listMenu(wallnames, ArraySize(wallnames), true, 2, "Change wallpaper");
}

// Function to show the main screen, root screen
void MainScreen() {
    Serial.println("MAINSCREEN");
    changeFont(0);
    drawWallpaper();
    drawStatusBar(true);
    // bool exit = false;
    int c = -1;
    while (1) {
        c = buttonsHelding();

        if ((c >= '0' || c == '*' || c == '#') && c <= '9') {
            numberInput(c);
            drawWallpaper();
        } else if (c == UP || c == SELECT)
            break;
    }

    currentScreen = SCREENS::MAINMENU;
}

void offlineCharging() {
    tft.fillRect(35, 100, 160, 80, 0xffff);
    tft.fillRect(195, 120, 10, 40, 0xFFFF);
    tft.setCursor(70, 310);
    while (true) {

        ulong mill = millis();
        if (buttonsHelding(false) != -1)
            return;
        int bp = getChargeLevel();
        tft.fillRect(40, 105, 150, 70, 0x0000);
        if (chrg.isChargerConnected() && bp < 1)
            while (millis() - mill < 500)
                if (buttonsHelding(false) != -1) {
                    return;
                }

        tft.fillRect(45, 110, 43, 60, 0xffff);
        mill = millis();
        if (chrg.isChargerConnected() && bp < 2)
            while (millis() - mill < 500)
                if (buttonsHelding(false) != -1) {
                    return;
                }
        tft.fillRect(45 + 48, 110, 43, 60, 0xffff);
        mill = millis();
        if (chrg.isChargerConnected() && bp < 3)
            while (millis() - mill < 500)
                if (buttonsHelding(false) != -1) {
                    return;
                }
        tft.fillRect(45 + 96, 110, 43, 60, 0xffff);

        while (chrg.getBatteryLevel() == 100)
            if (buttonsHelding(false) != -1) {
                return;
            }
    }
    tft.fillScreen(0x0000);
}

void incomingCall(Contact contact) {
    drawWallpaper();
    drawFromSd(0, 90, FULL_SCREEN_NOTIFICATION_IMAGE);
    changeFont(1);
    tft.setTextColor(0);
    tft.setCursor(15, 170);
    tft.print(contact.name);
    tft.setTextColor(0xf800);
    tft.setCursor(90, 140);
    changeFont(4);
    tft.setTextSize(1);
    tft.print("Ca l l ing");
    drawFromSd(45, 105, PHONE_ICON);

    writeCustomFont(55, 185, contact.phone, 1);
    drawFromSd(73, 90, LIGHTNING_ANIMATION[0]);
    int button = buttonsHelding();
    while (isCalling) {
        button = buttonsHelding();

        switch (button) {
        case ANSWER:
            if (sendATCommand("ATA").indexOf("NO CARRIER") == -1) {
                callActivity(contact);
            } else
                currentScreen = SCREENS::MAINSCREEN;
            break;
        case DECLINE:
            currentScreen = SCREENS::MAINSCREEN;
            return;
            break;

        default:
            break;
        }
    }
}
void makeCall(Contact contact) {
    if (!checkSim()) {
        return;
    }
    isAnswered = true;
    sendATCommand("ATD" + contact.phone + ";");
    callActivity(contact);
}
void callActivity(Contact contact) {
    ongoingCall  = true;
    //bool calling = true;
    tft.fillScreen(0);
    sBarChanged = true;
    drawStatusBar();

    changeFont(1);
    tft.setTextColor(0xffff);

    tft.setTextSize(1);
    tft.setCursor(20, 140 + 60);
    tft.print(contact.name);

    tft.setTextSize(2);
    tft.setCursor(0, 180 + 60);
    // tft.print(contact.number);
    writeCustomFont(5, 240, contact.phone);

    tft.setTextSize(1);
    tft.setCursor(85, 95);
    tft.print("Calling...");
    stateCall = GetState();
    delay(50);
    bool hang = false;
    while (stateCall == DIALING) {
        for (int i = 7; i >= 0; i--) {
            spinAnim(55, 60, 12, 6, i);
            delay(40);
            if (buttonsHelding() == DECLINE) {
                hang    = true;
                //calling = false;
                break;
            } else if (buttonsHelding() == UP) {
                //calling = false;
                break;
            }
        }
    }
    if (hang) {
        // if hang up
        sendATCommand("ATH");
        ongoingCall = false;

        return;
    }

    tft.fillScreen(0);
    sBarChanged = true;
    drawStatusBar();
    drawFromSd(40, 143, VOICE_ONLY_LABEL);
    while (stateCall != DISCONNECT) {
        if (buttonsHelding() == DECLINE) {
            sendATCommand("ATH");
            ongoingCall = false;
        }
        stateCall = GetState();
    }
    tft.fillScreen(0);
    drawStatusBar();
    ongoingCall = false;
    isAnswered  = false;
}

void contactss() {
    // if (!checkSim()) {
    //     currentScreen = SCREENS::MAINMENU;
    //     return;
    // }
    const String contmenu[] = {
        "Call",
        "Outgoing",
        "Edit",
        "Create",
        "Delete"};

    String contactNames[contactCount];
    for (int i = 0; i < contactCount; ++i) {
        contactNames[i] = contacts[i].name;
    }

    bool exit = false;
    while (!exit) {

        String contactNames[contactCount];
        for (int i = 0; i < contactCount; ++i) {
            contactNames[i] = contacts[i].name;
        }
        int selectedContactIndex = listMenu(contactNames, contactCount, false, 1, "Address Book");

        if (selectedContactIndex != -1) {
            int contextMenuSelection = -1;

            contextMenuSelection = choiceMenu(contmenu, 5, true);

            switch (contextMenuSelection) {

            case 0:
                // CALL
                makeCall(contacts[selectedContactIndex]);
                exit = true;
                break;
            case 1:
                // OUTGOING
                messageActivityOut(contacts[selectedContactIndex], "", "", true);
                break;
            case 2:
                // EDIT
                editContact(contacts[selectedContactIndex]);
                break;
            case 3:
                // CREATE

                editContact(Contact("", "", "", lastContactIndex + 1));
                break;
            case 4:
                // DELETE
                sendATCommand("AT+CPBW=" + String(contacts[selectedContactIndex].index));
                populateContacts();
                break;
            }

        } else {

            const String choice[1] = {"Create"};
            int          CMS       = choiceMenu({choice}, 1, true);
            if (!CMS) {
                editContact(Contact("", "", "", lastContactIndex + 1));
            } else
                exit = true;
        }
    }
    currentScreen = SCREENS::MAINMENU;
}

void inbox(bool outbox) {
    const char *title;
    if (outbox)
        title = "Outbox";
    else
        title = "Inbox";

    int      count    = 0;
    bool     exit     = false;
    Message *messages = nullptr;
    while (!exit) {
        exit = true;

        parseMessages(messages, count);
        mOption *a = new mOption[count];
        for (int i = 0; i < count; i++) {
            a[count - i - 1] = messages[i];
        }

        Serial.println("LISTMENU");
        int choice = -2;
        while (choice != -1) {
            choice = listMenu(a, count, false, 0, title);
            if (choice >= 0) {
                exit = !messageActivity(messages[count - choice - 1]);
                if (!exit)
                    choice = -1;
            }
        }
    }
}

String fileBrowser(File dir, String format, bool graphical) {
    SDImage folderIcon  = SDImage(0x663983, 18, 18, 0, true);
    SDImage fileIcon    = SDImage(0x663984 + (18 * 18 * 2), 18, 18, 0, true);
    String  currentPath = dir.path();
    File    file        = dir.openNextFile();
    dir.rewindDirectory();
    mOption *options = new mOption[64];
    int      choice  = -2;
    int      i       = 0;
    format.toLowerCase();
    do {
        i   = 0;
        dir = SD.open(currentPath);
        dir.rewindDirectory();
        file = dir.openNextFile();
        Serial.println(currentPath);
        while (file) {
            if (strlen(file.name()) != 0) {

                Serial.println();
                Serial.print(file.name());
                if (file.isDirectory()) {
                    Serial.print("/");

                    options[i].icon  = folderIcon;
                    options[i].label = file.name() + String("/");
                } else {
                    String fileLow = String(file.name());
                    fileLow.toLowerCase();
                    if (format == "*" || fileLow.endsWith(format)) {
                        options[i].icon  = fileIcon;
                        options[i].label = file.name();
                    } else
                        i--;
                }
                i++;
                file = dir.openNextFile();
            }
        }
        if (graphical)
            choice = listMenu(options, i, false, 2, "FILE MANAGER");
        else
            choice = listMenuNonGraphical(options, i, "FILE MANAGER");

        if (options[choice].label.endsWith("/")) {
            options[choice].label.remove(options[choice].label.lastIndexOf("/"));
            currentPath += "/" + options[choice].label;
        } else {
            currentPath += "/" + options[choice].label;
            return currentPath;
        }
        dir.close();
    } while (choice != -1);
    return "/null";
    file.close();
    dir.close();
}

// funny unstable thing, can corrupt your sdcard if interrupted
void downloadFile(const char *url, const char *path) {
    tft.fillScreen(0);
    tft.setCursor(0, 0);
    HTTPClient http;
    http.setTimeout(20000);
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        WiFiClient *stream = http.getStreamPtr();
        File        file   = SD.open(path, FILE_WRITE);

        if (file) {
            const uint16_t chunksize = 512;
            uint8_t        buffer[chunksize];
            uint32_t       downloaded = 0;
            ulong          startTime  = millis();

            Serial.println("Starting file download...");

            while (stream->connected() || stream->available()) {
                int bytesRead = stream->readBytes(buffer, sizeof(buffer));

                if (bytesRead > 0) {
                    file.write(buffer, bytesRead);
                    downloaded += bytesRead;

                    tft.print("Downloaded: ");
                    tft.print(downloaded);
                    tft.println(" bytes");
                    tft.setCursor(0, 0);

                    file.flush();
                } else if (bytesRead == 0) {
                    break;
                } else {
                    Serial.println("Read error or disconnected");
                    break;
                }
            }

            float elapsedTime = (millis() - startTime) / 1000.0;
            tft.println("Download complete.");
            tft.print("Total bytes downloaded: ");
            tft.println(downloaded);
            tft.print("Elapsed time: ");
            tft.print(elapsedTime);
            tft.println(" seconds");
            tft.print("Download speed: ");
            tft.print(downloaded / elapsedTime);
            tft.println(" bytes/second");

            file.close();
        } else {
            Serial.println("Failed to open file on SD card.");
        }
    } else {
        Serial.printf("Download failed, HTTP error code: %d\n", httpCode);
    }

    http.end();
}

void mediaPlayer(String path) {
    // TODO
}

void ringtoneSelector(bool isMail) {
    File dir = SD.open("/AUDIO");
    if (!dir) {
        ErrorWindow("NO /AUDIO");
        return;
    }

    dir.rewindDirectory();
    int  count = 0;
    File entry;

    while (entry = dir.openNextFile()) {
        if (!entry.isDirectory()) {
            count++;
        }
    }

    if (count == 0) {
        sysError("No ringtones found in /AUDIO");
        return;
    }

    String  pathes[count];
    mOption opt[count];
    int     iconIndex = -1;

    dir.rewindDirectory();
    String *compareRingtone = isMail ? &currentMailRingtonePath : &currentRingtonePath;

    count = 0;
    while (entry = dir.openNextFile()) {
        if (!entry.isDirectory() && String(entry.name()).endsWith(".SGAUDIO")) {
            pathes[count]    = entry.path();
            opt[count].label = entry.name();
            opt[count].label.replace(".SGAUDIO", "");

            if (pathes[count].equals(*compareRingtone)) {
                opt[count].icon = mailimg[0]; // REPLACE WITH NOTE ICON
                iconIndex       = count;
            }
            count++;
        }
    }

    int choice = 0;
    while (choice != -1) {
        choice = listMenu(opt, count, false, 2, "Set ringtone", true, choice);
        if (choice < 0)
            return;
        if (iconIndex >= 0)
            opt[iconIndex].icon = SDImage();
        opt[choice].icon = mailimg[0];
        iconIndex        = choice;
        *compareRingtone = pathes[choice];
    }
}

bool messageActivity(Contact contact, String date, String subject, String content, int index, bool outcoming, bool sms) {
    tft.setTextWrap(true);
    // returns if deleted
    if (sms) {
        String response = sendATCommand("AT+CMGR=" + String(index));
        if (response.indexOf("OK") == -1) {
            content = "======\nERROR LOADING FOLLOWING MESSAGE\n======";
        }
        int resIndex = 0;
        for (int i = 0; i < 7; i++) {
            resIndex = response.indexOf('\"', ++resIndex);
        }
        int endIndex = response.indexOf('\"', ++resIndex);
        Serial.println("resIndex:" + String(resIndex) + "\nendIndex:" + endIndex);
        date = response.substring(resIndex, endIndex);
        date.replace(',', ' ');
        date = date.substring(0, date.lastIndexOf(':'));
        Serial.println(date);
        content = response.substring(endIndex + 3, response.lastIndexOf("OK") - 2);
    }
    content.trim();
    const String choices[4] = {"Reply", "Return", "Delete", "HEX to ASCII"};
    drawStatusBar();

    drawFromSd(0, 26, BLUEBAR_IMAGE);
    drawFromSd(0, 26, BLUEBAR_ICONS[0]);
    int y_jump = 22;
    int y_scr  = 0;
    int y_text = 18;

    tft.setCursor(30, 45);
    tft.setTextSize(1);
    changeFont(1);
    tft.setTextColor(0xffff);
    bool deleted = false;
    tft.print("Recieve Mail");
    tft.setTextColor(0);
    tft.setViewport(0, 51, 240, 269, true);
    bool exit = false;
    while (!exit) {
        deleted = false;
        tft.setTextColor(0);
        drawFromSd(0, 0, BACKGROUND_IMAGE_CUTTED);
        // tft.fillScreen(0xFFFF);
        drawFromSd(0, 0 + y_scr, in_mail[0]);
        tft.setCursor(24, 0 + y_text + y_scr);
        tft.println(date);

        drawFromSd(0, 24 + y_scr, in_mail[1]);
        tft.setCursor(24, 24 + y_text + y_scr);
        tft.println(!contact.name.isEmpty() ? contact.name : !contact.phone.isEmpty() ? contact.phone
                                                         : !contact.email.isEmpty()   ? contact.email
                                                                                      : "UNKNOWN");
        content = SplitString(content);
        tft.drawLine(0, 48 + y_scr, 240, 48 + y_scr, 0);
        int height = measureStringHeight(content);
        tft.print(content);
        int ch = -2;
        int r  = -1;
        while (r == -1) {
            r = buttonsHelding();
            switch (r) {
            case DOWN:
                if (y_scr > -height)
                    y_scr -= y_jump;
                break;
            case UP:
                if (y_scr < 0)
                    y_scr += y_jump;
                break;
            case BACK:
                ch = choiceMenu(choices, 4, true);
                switch (ch) {
                case -1:
                    exit = true;
                    break;
                case 0:
                    messageActivityOut(contact, "", "", true);
                    break;

                case 2:
                    if (confirmation("ARE YOU SURE YOU WANT TO DELETE MESSAGE?")) {
                        sendATCommand("AT+CMGD=" + String(index), 5000);
                        exit    = true;
                        deleted = true;
                    }
                    break;
                case 3:
                    content = HEXTOASCII(content);
                    break;
                }
                break;
            default:
                break;
            }
        }
        tft.resetViewport();

        tft.setViewport(0, 51, 240, 269, true);
    }
    tft.resetViewport();
    return deleted;
}

bool messageActivity(Message message) {
    return messageActivity(message.contact, message.date, message.subject, message.content, message.index, false, true);
}

void getCharacterPosition(String str, int &x, int &y, int &index, int direction = 0, int screenWidth = 240) {
    // if you see that function overflowed with comments its means AI was used...

    x                            = 0;  // Current X position
    y                            = 0;  // Current Y position
    int              charCount   = 0;  // Global character count (including wrapped lines)
    String           currentLine = ""; // Accumulating the current line
    //int              lineStart   = 0;  // Store the starting index of the current line
    //int              lineEnd     = 0;  // Store the end of the current line for wrapping
    std::vector<int> lineStarts;       // To store the starting index of each line

    // Step 1: Process the string character by character
    for (int i = 0; i < str.length(); i++) {
        char currentChar = str[i];

        // If it's a newline character, handle the line break
        if (currentChar == '\n') {
            // Update Y position and store the start index for the current line
            y += tft.fontHeight();       // Move to the next line
            lineStarts.push_back(i + 1); // Store the start index of the next line
           // lineStart   = i + 1;         // Start of the next line
            //lineEnd     = i;             // End of the current line
            currentLine = "";            // Reset the current line
        } else {
            // If we haven't reached the wrapping width, add the character to the current line
            String tempLine  = currentLine + currentChar;
            int    lineWidth = tft.textWidth(tempLine);

            if (lineWidth > screenWidth) {
                // If the line exceeds the screen width, wrap to the next line
                y += tft.fontHeight();             // Move to the next line
                lineStarts.push_back(i);           // Store the start index of the next line
                currentLine = String(currentChar); // Start a new line with this character
            //    lineStart   = i;                   // Start of the new line
             //   lineEnd     = i - 1;               // End of the previous line
            } else {
                // Otherwise, keep adding characters to the current line
                currentLine = tempLine;
            }
        }

        // Adjust the index based on direction
        if (direction == 1 && charCount > index) // Moving up (previous line)
        {
            if (lineStarts.size() > 1) {
                index = lineStarts[lineStarts.size() - 2]; // Set to the start of the previous line
                break;                                     // Stop here as we've adjusted the index
            }
        } else if (direction == 2 && charCount < index) // Moving down (next line)
        {
            if (lineStarts.size() > 0 && lineStarts.size() - 1 < lineStarts.size()) {
                index = lineStarts[lineStarts.size() - 1]; // Set to the start of the next line
                break;                                     // Stop here as we've adjusted the index
            }
        }

        // If the current character is the one we are looking for, calculate its position
        if (charCount == index) {
            // Use a while loop to calculate the exact position based on the substring width
            int    b             = 0;           // Initialize a variable to track the length of the substring
            String lineSubstring = currentLine; // Substring of the current line

            // Keep incrementing b until the substring width exceeds the screen width
            while (b < lineSubstring.length() && tft.textWidth(lineSubstring.substring(0, b)) <= screenWidth) {
                b++; // Increment until the condition is met
            }

            // Now set x to the calculated width
            x = tft.textWidth(lineSubstring.substring(0, b));
            return; // Exit as we found the desired position
        }

        charCount++; // Increment global character count
    }

    // If the index is beyond the last character, calculate the position at the end of the last line
    x = tft.textWidth(currentLine);
}

void messageActivityOut(Contact contact, String subject, String content, bool sms) {
    // TEXT LIMIT VERTICAL VIEWPORT
#define TLVP 238
    // (goddamit I forgot what this variable does, for future: do proper naming for variables and defines, that's pro tip.)
    // I remembered that its just size of accessible height of current viewport

    int limit = 0;
    if (sms) {
        limit = 160;
    } else {
        limit = 400;
    }
    String messagebuf;
    int    curx     = 0;
    int    cury     = 0;
    int    text_pos = 0;
    int    position = 0;
    drawStatusBar();
    SDImage in_mail[4] = {
        SDImage(0x663E91, 23, 24, 0, false),
        SDImage(0x663E91 + (23 * 24 * 2), 23, 24, 0, false),
        SDImage(0x663E91 + (23 * 24 * 2 * 2), 23, 24, 0, false),
        SDImage(0x663E91 + (23 * 24 * 2 * 3), 23, 24, 0, false),
    };
    drawFromSd(0, 26, BLUEBAR_IMAGE);
    drawFromSd(0, 26, BLUEBAR_ICONS[0]);
    int y_jump = 22;
    int y_scr  = 0;
    int y_text = 18;
    int min_y  = y_scr;
    tft.setCursor(30, 45);
    tft.setTextSize(1);
    changeFont(1);
    tft.setTextColor(0xffff);

    tft.print("Outgoing Mail");
    tft.setTextColor(0);

    tft.setViewport(0, 51, 240, 269, true);
    bool exit = false;
    while (!exit) {
        position = 0;
        drawFromSd(0, 0, BACKGROUND_IMAGE_CUTTED);
        // tft.fillScreen(0xFFFF);
        // position += 24;
        drawFromSd(0, position + y_scr, in_mail[2]);
        tft.setCursor(24, position + y_text + y_scr);
        tft.println(!contact.name.isEmpty() ? contact.name : !contact.phone.isEmpty() ? contact.phone
                                                         : !contact.email.isEmpty()   ? contact.email
                                                                                      : "UNKNOWN");
        messagebuf = SplitString(messagebuf);
        if (!subject.isEmpty() || !sms) {
            position += 24;
            drawFromSd(0, position + y_scr, in_mail[3]);
            tft.setCursor(24, position + y_text + y_scr);
            tft.println(subject);
        }
        position += 24;
        tft.drawLine(0, position + y_scr, 240, position + y_scr, 0);
        tft.print(messagebuf);
        int    input   = -1;
        String a[] = {"Return", "Send Message", "Delete", "Save To Drafts"};
        int    u;
        tft.drawLine(curx + 1, 2 + position + y_scr + cury, 1 + curx, y_scr + cury + position + 20, TFT_BLACK);
        Serial.println("CURX:" + String(curx) + " CURY:" + String(cury));
        while (input == -1) {

            if (y_scr < min_y) {
                min_y = y_scr;
            }
            input = buttonsHelding();
            switch (input) {
            case DOWN:
                // if (y_scr > -height)
                //   y_scr -= y_jump;
                text_pos = findCharIndex(messagebuf, text_pos, DOWN);
                findSplitPosition(messagebuf, text_pos, curx, cury);
                break;
            case UP:
                // if (y_scr < 0)
                //   y_scr += y_jump;
                text_pos = findCharIndex(messagebuf, text_pos, UP);
                findSplitPosition(messagebuf, text_pos, curx, cury);
                break;
            case RIGHT:
                if (text_pos >= 0)
                    findSplitPosition(messagebuf, ++text_pos, curx, cury);

                break;
            case LEFT:
                if (text_pos > 0)
                    findSplitPosition(messagebuf, --text_pos, curx, cury);

                break;
            case BACK:

                u = choiceMenu(a, 4, true);
                tft.setTextColor(0);
                switch (u) {
                case -1:
                    input    = -2;
                    exit = true;
                    break;
                case 1:
                    ESP_LOGI("INFO","SEND MESSAGE");
                    if (sendATCommand("AT+CMGF=1").indexOf("OK") != -1) {
                        sendATCommand("AT+CMGS=\"" + contact.phone + "\"");
                        sendATCommand(messagebuf + char(26));
                    }
                    tft.resetViewport();
                    return;
                    break;
                case 2:
                    ESP_LOGI("INFO","DELETE");

                    break;
                case 3:
                    ESP_LOGI("INFO","Save to Drafts");
                    break;

                default:
                    break;
                }
                break;
            default:
                if (input >= '0' && input <= '9') {

                    char l = textInput(input);
                    input      = buttonsHelding();
                    if (l != 0) {

                        //ESP_LOGI("INFO","Y:" + String(tft.getCursorY()));
                        // drawCutoutFromSd(SDImage(0x639365, 240, 269, 0, false), 0, 260, 120, 20, 0, 240);

                        if (messagebuf.length() < limit)
                            if (l != '\r') {
                                if (l != '\b') {

                                    messagebuf = messagebuf.substring(0, text_pos) + l + messagebuf.substring(text_pos, messagebuf.length());
                                    text_pos++;
                                } else {
                                    messagebuf = messagebuf.substring(0, text_pos - 1) + messagebuf.substring(text_pos, messagebuf.length());
                                    input          = BACK;
                                }
                            }
                        if (tft.getCursorY() > TLVP) {
                            y_scr -= y_jump;
                            if (y_scr < min_y)
                                min_y = y_scr;
                            input = BACK;
                        }

                        if (y_scr != min_y) {
                            y_scr = min_y;
                            input     = BACK;
                        }
                        //ESP_LOGI("INFO","MINIMUM:" + String(y_scr));
                        findSplitPosition(messagebuf, text_pos, curx, cury);
                        input = BACK;
                    }
                }
                break;
            }
        }
        tft.resetViewport();

        tft.setViewport(0, 51, 240, 269, true);
    }
    tft.resetViewport();
}

void editContact(Contact contact) {
    int pos       = 0;
    int textboxes = 2;
    int buttons   = 2;
    int direction;
    drawFromSd(0, 26, BLUEBAR_IMAGE);
    drawFromSd(0, 26, BLUEBAR_ICONS[1]);
    drawStatusBar();
    tft.setCursor(30, 45);
    tft.setTextSize(1);
    changeFont(1);
    tft.setTextColor(0xffff);
    String boxString[textboxes] = {contact.name, contact.phone};
    tft.print("Edit Contact");
    drawFromSd(0, 51, BACKGROUND_IMAGE_CUTTED);
    InputField("Name", contact.name, 70, true, false, false);
    InputField("Phone Number", contact.phone, 120, true, false, false);

    button("SAVE", 10, 285, 100, 28);
    button("CANCEL", 120, 285, 100, 28);
    bool save   = false;
    bool cancel = false;
    bool exit   = false;
    while (!exit) {
        switch (pos) {
        case 0:
            boxString[pos] = InputField("Name", boxString[pos], 70, false, false, true, &direction);
            break;
        case 1:
            boxString[pos] = InputField("Phone Number", boxString[pos], 120, false, false, true, &direction, true);
            break;
        case 2:
            save = button("SAVE", 10, 285, 100, 28, true, &direction);
            if (save) {
                if (boxString[1].isEmpty())
                    break;
                if (boxString[0].isEmpty()) {
                    boxString[0] = boxString[1];
                }

                sendATCommand("AT+CPBS=\"SM\"");
                String request = "AT+CPBW=" +
                                 String(contact.index) + ",\"" +
                                 boxString[1] + "\"," +
                                 String(boxString[1].indexOf("+") == 0 ? 145 : 129) +
                                 ",\"" + boxString[0] + "\"";
                String result = sendATCommand(request);
                populateContacts();
                Serial.println(result);
                return;
            }

            // boxString[pos] = textbox("E-mail", boxString[pos], 170, false, false, true, &direction);
            break;
        case 3:
            cancel = button("CANCEL", 120, 285, 100, 28, true, &direction);
            if (cancel)
                return;
            break;
        default:
            pos = 0;
            break;
        }
        Serial.println("DIRECTION:" + String(direction));
        Serial.println("POS:" + String(pos));
        switch (direction) {
        case DOWN:
            if (pos < textboxes)
                pos++;
            else if (pos < textboxes + buttons)
                ;
            else
                pos = 0;
            break;
        case UP:
            if (pos > textboxes)
                pos = textboxes - 1;

            else if (pos > 0)
                pos--;

            break;
        default:
            break;
        case RIGHT:
            if (pos < textboxes + buttons - 1)
                pos++;
            break;
        case LEFT:
            if (pos > textboxes)
                pos--;
            break;
        }
    }
}

void imageViewer() {
    // TODO
    tft.setViewport(0, 51, 240, 269, true);
    tft.fillScreen(0);

    // CODE
    tft.resetViewport();
}

void recovery(String message) {

    tft.setCursor(0, 40);
    tft.fillScreen(0);
    tft.setTextFont(1);
    tft.setTextSize(4);
    tft.setTextColor(0x00FF);
    tft.println("=RECOVERY=\n");
    tft.setTextSize(1);
    tft.setTextColor(0xFFFF);
    tft.println(message);
    mOption options[2] = {{"Choose resource file"}, {"Try again"}};
    int     choice     = listMenuNonGraphical(options, ArraySize(options), "Choose action.", 150);
    switch (choice) {
    case 0:
        String TempResPath = fileBrowser(SD.open("/"), ".SG", false);

        File   file = SD.open(TempResPath);
        String buf;
        file.seek(0);
        for (int i = 0; i < 12; i++) {
            buf += (char)file.read();
        }
        /*
         * Fun fact, if you will open any IMAGES.SG in any
         * text editor you'll see that there ELPSYKONGROO at the beginning.
         * That's to prevent wrong(?) files.
         */

        if (buf.indexOf("ELPSYKONGROO") != -1) {
            mOption optionss[2] = {{"Yes!"}, {"No..."}};
            choice              = listMenuNonGraphical(optionss, 2, "Are you sure you want to change files?");
            if (choice == 0) {
                resPath = TempResPath;
                preferences.begin("settings", false);
                preferences.putString("resPath", resPath);
                preferences.end();
            }
        } else {
            tft.fillScreen(0);
            tft.setCursor(0, 0);
            tft.println("File is invalid!\nPress any key to continue...");
            while (buttonsHelding(false) == -1)
                ;
        }
        break;
    }
}

void numberInput(char first) {
    tft.fillRect(0, 300, 240, 20, 0);
    sBarChanged = true;
    drawStatusBar();
    const uint8_t max_char = 13;
    String        number;
    number += first;
    char c   = 255;
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
    //int  sizes[12];
    char result = 0;
    int  pos = 0;
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
        //sizes[i] = b;
        // Serial.println(b, DEC);
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
        if (c == input || first){
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
    if (viewport){
        tft.setViewport(vx, vy, w, h);
    }
    if (result == '\r') {
        return 0;
    }
    return result;
}

void LockScreen() {
    Serial.println("LockScreen");
    sBarChanged    = true;
    isScreenLocked = true;
    drawStatusBar();
    ulong mill;
    bool  exit = false;
    while (!exit) {
        mill = millis();
        while (buttonsHelding() == '*') {
            if (millis() > mill + 1000) {
                exit      = true;
                millSleep = millis();
                break;
            }
        }
    }
    isScreenLocked = false;
    sBarChanged    = true;
    drawStatusBar();
}

void WifiConnect(String ssid, String password) {
}

void WiFiList() {
    fastMode(true);
    WiFi.begin();
    WiFi.mode(WIFI_STA);
    while (true) {
        int      count = WiFi.scanNetworks();
        String   names[50];
        uint8_t  a;
        uint8_t *l;
        int32_t  c, d;
        for (int i = 0; i < count; i++) {
            WiFi.getNetworkInfo(i, names[i], a, c, l, d);
        }
        int ch = listMenu(names, count, false, 0, "WI-FI");
        if (ch == -1)
            return;
        else {
            // TODO: Actually Connect to WiFi

            return;
        }
    }
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
