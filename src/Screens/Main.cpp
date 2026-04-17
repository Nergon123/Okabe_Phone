#include "Main.h"
#include <System/LanguageSystem.h>

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
    currentRenderTarget->present();
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
            currentRenderTarget->present();
            break;
        }

        case UP: {
            old_choice = choice;
            choice -= 2;
            rendermenu(choice, old_choice);
            currentRenderTarget->present();
            break;
        }

        case DOWN: {
            old_choice = choice;
            choice += 2;
            rendermenu(choice, old_choice);
            currentRenderTarget->present();
            break;
        }
        case LEFT: {
            old_choice = choice;
            choice--;
            rendermenu(choice, old_choice);
            currentRenderTarget->present();
            break;
        }

        case RIGHT: {
            old_choice = choice;
            choice++;
            rendermenu(choice, old_choice);
            currentRenderTarget->present();
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
    currentRenderTarget->present();
    while (1) {
        int button = buttonsHelding();
        if ((button >= '0' && button <= '9') || button == '*' || button == '#') {
            numberInput(button);
            drawWallpaper();
            currentRenderTarget->present();
        }
        else if (button == UP || button == SELECT) { break; }
    }
    currentScreen = SCREENS::MAINMENU;
}

// @param level available levels: `0 1 2 3`, where 0 is empty
void drawLevelCharge(uint8_t level) {
    if (level > 3) { return; }

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
            ulong mill = hw->millis();
            drawLevelCharge(i);
            currentRenderTarget->present();
            while (hw->millis() - mill < 500 || i == 3) {
                if (buttonsHelding(false) != -1) { return; }
            }
        }
    }
    tft.fillScreen(0x0000);
    currentRenderTarget->present();
}

/*
 *Function to show recovery screen
 *@param message message to be displayed
 */
void recovery(NString message) {
    if (res.Files) {
        free(res.Files);
        delete res.Files;
    }
    res.Files = nullptr;
    while (!res.Files) {
        tft.setCursor(0, 40);
        tft.fillScreen(0);
        tft.setTextFont(1);
        tft.setTextSize(4);
        tft.setTextColor(0x00FF);
        tft.println(getTranslation(TextKey::RECOVERY_TITLE));
        tft.setTextSize(1);
        tft.setTextColor(0xFFFF);
        tft.println(message);
        std::vector<mOption> options = {{getTranslation(TextKey::RECOVERY_OPT_CHOOSE_RES)}, {getTranslation(TextKey::RECOVERY_TRY_AGAIN)}};
        int choice = listMenuNonGraphical(options, options.size(), getTranslation(TextKey::RECOVERY_TITLE_CHOOSE_ACT), 150);
        switch (choice) {
        case 0:
            NString TempResPath = fileBrowser("/", "|.nph|.npz|", false);
            resPath             = TempResPath;
            NFile *nFile        = VFS.open(resPath);
            res.Init(nFile);
            res.CopyToRam();

            if (res.Files) {
                preferences.begin("System");
                preferences.putString("resPath", resPath.c_str());
                preferences.end();
                return;
            }
            break;
        }
    }
}

// Function to lock keypad
void LockScreen() {
#ifdef lockscreen
    isScreenLocked = true;
    drawStatusBar(true);
    ulong mill;
    bool  exit = false;
    while (!exit) {
        mill = hw->millis();
        while (buttonsHelding() == '*') {
            if (hw->millis() > mill + 1000) {
                exit      = true;
                millSleep = hw->millis();
                break;
            }
        }
    }
    isScreenLocked = false;
    drawStatusBar(true);
#else
#warning LockScreen is disabled.
#endif
}
