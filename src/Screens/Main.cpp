#include "Main.h"

// Current screen selection based on currentScreen variable
void screens() {
    switch (currentScreen) {
    case MAINSCREEN: MainScreen(); break;
    case MAINMENU: MainMenu(); break;
    case MESSAGES: messages(); break;
    case CONTACTS: contactss(); break;
    case SETTINGS: settings(); break;
    case E: e(); break;
    }
}

// ## Render menu
//  This function renders the menu icons on the screen.
void rendermenu(int &choice, int old_choice) {
    uint8_t menuEntries[4] = {R_MENU_MAIL_ICON, R_MENU_CONTACTS_ICON, R_MENU_E_ICON,
                              R_MENU_SETTINGS_ICON};

    if (choice > 3) { choice = 0; }
    if (choice < 0) { choice = 3; }

    if (old_choice != choice) {
        res.DrawImage(menuEntries[old_choice]);
        res.DrawImage(menuEntries[choice], 1);
    }
}

// Function to show the main menu
void MainMenu() {
    res.DrawImage(R_MENU_BACKGROUND);
    res.DrawImage(R_MENU_MAIL_ICON, 1);
    int choice = 0;
    for (int i = 0; i < 4; i++) { rendermenu(choice, i); }
    int old_choice = 0;
    while (true) {
        switch (buttonsHelding()) {
        case BACK: currentScreen = SCREENS::MAINSCREEN; return;
        case SELECT: {
            switch (choice) {
            case 0: currentScreen = SCREENS::MESSAGES; return;
            case 1: currentScreen = SCREENS::CONTACTS; return;
            case 2: currentScreen = SCREENS::E; return;
            case 3: currentScreen = SCREENS::SETTINGS; return;
            }
            res.DrawImage(R_MENU_BACKGROUND);
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
        if (currentScreen == MAINSCREEN) { return; }
    }
}

// Function to show the main screen, root screen
void MainScreen() {
    drawWallpaper();
    changeFont(0);
    drawStatusBar(true);
    while (1) {
        int button = buttonsHelding();
        if ((button >= '0' && button <= '9') || button == '*' || button == '#') {
            numberInput(button);
            drawWallpaper();
        }
        else if (button == UP || button == SELECT) { break; }
    }
    currentScreen = SCREENS::MAINMENU;
}

// @param level available levels: `0 1 2 3`, where 0 is empty
void drawLevelCharge(uint8_t level) {
    if (level < 0 || level > 3) { return; }

    if (level == 0) { tft.fillRect(40, 105, 150, 70, 0x0000); }
    else { tft.fillRect(45 + 48 * (level - 1), 110, 43, 60, 0xffff); }
}
// Function to show battery when its conected to charger and "powered off"
void offlineCharging() {
    tft.fillRect(35, 100, 160, 80, 0xffff);
    tft.fillRect(195, 120, 10, 40, 0xFFFF);
    int maxLevel = 0;
    while (true) {
        if (getChargeLevel() > maxLevel) { maxLevel = getChargeLevel(); }
        for (int i = 0; i <= maxLevel; i++) {
            ulong mill = millis();
            drawLevelCharge(i);
            while (millis() - mill < 500 || i == 3) {
                if (buttonsHelding(false) != -1) { return; }
            }
        }
    }
    tft.fillScreen(0x0000);
}

/*
 *Function to show recovery screen
 *@param message message to be displayed
 */
void recovery(NString message) {
    initSDCard(true);
    res.Files[RES_MAIN] = nullptr;
    while (!res.Files[RES_MAIN]) {
        tft.setCursor(0, 40);
        tft.fillScreen(0);
        tft.setTextFont(1);
        tft.setTextSize(4);
        tft.setTextColor(0x00FF);
        tft.println("=RECOVERY=\n");
        tft.setTextSize(1);
        tft.setTextColor(0xFFFF);
        tft.println(message);
        std::vector<mOption> options = {{"Choose resource file"}, {"Try again"}};
        int     choice = listMenuNonGraphical(options, options.size(), "Choose action.", 150);
        switch (choice) {
        case 0:
            NString TempResPath = fileBrowser("/", ".nph", false);
            resPath             = TempResPath;
            NFile *nFile        = VFS.open(resPath);
            res.Init(nFile); 
            res.CopyToRam(RES_MAIN);
            if (res.Files[RES_MAIN]) { return; }
            break;
        }
    }
}

// Function to lock keypad
void LockScreen() {
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
