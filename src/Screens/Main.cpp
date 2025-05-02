#include "Main.h"


void screens() {
    switch (currentScreen) {
    case SCREENS::MAINSCREEN:
        MainScreen();
        break;
    case SCREENS::MAINMENU:
        MainMenu();
        break;
    case SCREENS::MESSAGES:
        messages();
        break;
    case SCREENS::CONTACTS:
        contactss();
        break;
    case SCREENS::SETTINGS:
        settings();
        break;
    case SCREENS::E:
        e();
        break;
    }
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


// Function to show the main screen, root screen
void MainScreen() {
    Serial.println("MAINSCREEN");
    changeFont(0);
    drawWallpaper();
    drawStatusBar(true);
    // bool exit = false;
    int currentButton = -1;
    while (1) {
        currentButton = buttonsHelding();

        if ((currentButton >= '0' || currentButton == '*' || currentButton == '#') && currentButton <= '9') {
            numberInput(currentButton);
            drawWallpaper();
        } else if (currentButton == UP || currentButton == SELECT)
            break;
    }

    currentScreen = SCREENS::MAINMENU;
}

void offlineCharging() {
    tft.fillRect(35, 100, 160, 80, 0xffff);
    tft.fillRect(195, 120, 10, 40, 0xFFFF);
    tft.setCursor(70, 310);
    int chargeIndexOld = 0;
    while (true) {

        ulong mill = millis();
        if (buttonsHelding(false) != -1)
            return;
        int chargeIndexNew = getChargeLevel();
        if (chargeIndexNew > chargeIndexOld) {
            chargeIndexOld = chargeIndexNew;
        }
        tft.fillRect(40, 105, 150, 70, 0x0000);
        if (chrg.isChargerConnected() && chargeIndexOld < 1)
            while (millis() - mill < 500)
                if (buttonsHelding(false) != -1) {
                    return;
                }

        tft.fillRect(45, 110, 43, 60, 0xffff);
        mill = millis();
        if (chrg.isChargerConnected() && chargeIndexOld < 2)
            while (millis() - mill < 500)
                if (buttonsHelding(false) != -1) {
                    return;
                }
        tft.fillRect(45 + 48, 110, 43, 60, 0xffff);
        mill = millis();
        if (chrg.isChargerConnected() && chargeIndexOld < 3)
            while (millis() - mill < 500)
                if (buttonsHelding(false) != -1) {
                    return;
                }
        tft.fillRect(45 + 96, 110, 43, 60, 0xffff);
        mill = millis();
        while (millis() - mill < 500)
            if (buttonsHelding(false) != -1) {
                return;
            }
        while (chrg.getBatteryLevel() == 100)
            if (buttonsHelding(false) != -1) {
                return;
            }
    }
    tft.fillScreen(0x0000);
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

