#include "Settings.h"

const int lastImage = 42;

void graphTest() {
#warning graphTest not implemented
    int page   = 0;
    int button = -1;
    tft.fillScreen(0);
    while (true) {
        switch (page) {
        case 0: break;
        }
        button = buttonsHelding();

        switch (button) {
        case LEFT: break;
        case RIGHT: break;
        case BACK: return;
        }
    }
}

void debugMenu() {
#warning debugMenu not implemented
    NString dbgMenu[] = {"Graphics Test"};
    int     choice    = LISTMENU_NULL;
    while (choice < 0) {
        choice = choiceMenu(dbgMenu, ArraySize(dbgMenu), false);
        switch (choice) {
        case 0: graphTest(); break;
        }
    }
};

void advancedSettings() {
#warning advancedSettings not implemented
    NString options[] = {
        "System",
        "Connectivity",
        "Look and feel",
        "Experimental",
    };
    int     lookAndFeel   = LISTMENU_NULL;
    int     menuSelection = LISTMENU_NULL;
    NString laf_opts[]    = {"Change Theme"};

    while (menuSelection != -1) {
        res.DrawImage(R_MENU_BACKGROUND);
        res.DrawImage(R_SETTING_MENU_L_HEADER);

        menuSelection = choiceMenu(options, ArraySize(options), false);
        switch (menuSelection) {
        case 2: lookAndFeel = choiceMenu(laf_opts, ArraySize(laf_opts), false); break;

        default: return;
        }
    }
}

// Function to show the settings menu
// This function is called when the user wants to change settings
// It allows the user to change the wallpaper, ringtones, etc.
void settings() {
    NString settingsOptions[] = {"Change Wallpaper", "Set call ringtone", "Set mail ringtone",
                                 "Advanced Settings"};
    int     menuSelection     = -2;
    while (menuSelection != -1) {
        res.DrawImage(R_MENU_BACKGROUND);
        res.DrawImage(R_SETTING_MENU_L_HEADER);
        menuSelection        = choiceMenu(settingsOptions, ArraySize(settingsOptions), false);
        int pictureIndex     = -1;
        int confPictureIndex = -2;

        switch (menuSelection) {
        case -1: currentScreen = SCREENS::MAINMENU; return;

        case 0:
            pictureIndex = gallery();

            if (pictureIndex == -1) {
                currentScreen = SCREENS::MAINMENU;
                return;
            }
            break;
        case 1: ringtoneSelector(false); break;
        case 2: ringtoneSelector(true); break;
        case 3: advancedSettings(); break;
        default:
            currentScreen = SCREENS::MAINMENU;
            return;
            break;
        }
        NString GalleryChoice[] = {"Preview", "Confirm"};
        if (pictureIndex != -1 && pictureIndex != lastImage) {

            confPictureIndex = choiceMenu(GalleryChoice, 2, true);
            switch (confPictureIndex) {
            case 0:
                // drawImage(WALLPAPER_IMAGES_BASE.address + WALLPAPER_IMAGES_MULTIPLIER *
                // pictureIndex, 0, 26, WALLPAPER_IMAGES_BASE.w, WALLPAPER_IMAGES_BASE.h, "/" +
                // resPath, 0, 0);
                tft.fillScreen(0);
                tft.setTextColor(TFT_RED);
                tft.print("wallpapers are gone...");
                while (buttonsHelding() != BACK);
                break;
            case 1:
                preferences.begin("settings", false);
                preferences.putInt("wallpaperIndex", pictureIndex);
                preferences.putString("wallpaper", "");
                preferences.end();
                wallpaperIndex = pictureIndex;
                // TODO WALLPAPER
                currentWallpaperPath = "";
                currentScreen        = SCREENS::MAINSCREEN;
                return;
                break;
            default: break;
            }
        }
        else if (pictureIndex == lastImage) {
            NString path = fileBrowser("/", ".png");
            if (VFS.exists(path)) { confPictureIndex = choiceMenu(GalleryChoice, 2, true); }
            else { break; }
            switch (confPictureIndex) {
            case 0: break;
            case 1:
                preferences.begin("settings", false);
                preferences.putInt("wallpaperIndex", -1);
                preferences.putString("wallpaper", path.c_str());
                preferences.end();
                // TODO WALLPAPER
                currentWallpaperPath = path;
                currentScreen        = SCREENS::MAINSCREEN;
                return;
                break;
            default: break;
            }
        }
    }
    currentScreen = SCREENS::MAINMENU;
}

// Function to show the gallery menu
//
// This function is called when the user wants to change the wallpaper
//
// If "Pick wallpaper..." is selected, it allows the user to select a custom wallpaper
// @return The index of the selected wallpaper
int gallery() {
    if (!VFS.exists("/sd" + resPath)) { return lastImage; }
    std::vector<mOption> wallpaperOptions = {mOption("Pick wallpaper...", Image())};
    return listMenu(wallpaperOptions, wallpaperOptions.size(), true, LM_SETTINGS,
                    "Change wallpaper");
}

// Set time screen
// This function is called when the user wants to set the time
// @param time Pointer to the time_t variable
void setTime(time_t *time) {
    res.DrawImage(R_LIST_MENU_BACKGROUND);
    res.DrawImage(R_LIST_HEADER_BACKGROUND);
    res.DrawImage(R_LIST_HEADER_ICONS, 2);
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

    tm  tm_time   = *gmtime(time);
    int temp_year = 1900 + tm_time.tm_year;

    int  choice    = 0;
    bool exit      = false;
    bool renderall = true;
    int  direction = LEFT;
    while (!exit) {

        sNumberChange(57, 90, 25, 25, tm_time.tm_mday, 1, 31, choice == 0 && !renderall,
                      &direction);
        sNumberChange(93, 90, 25, 25, tm_time.tm_mon, 1, 12, choice == 1 && !renderall,
                      &direction);
        sNumberChange(129, 90, 50, 25, temp_year, 1900, 2100, choice == 2 && !renderall,
                      &direction);
        sNumberChange(83, 170, 25, 25, tm_time.tm_hour, 0, 23, choice == 3 && !renderall,
                      &direction);
        sNumberChange(117, 170, 25, 25, tm_time.tm_min, 0, 59, choice == 4 && !renderall,
                      &direction);
        bool confirm = button("CONFIRM", 10, 280, 100, 30, choice == 5 && !renderall, &direction);
        bool cancel  = button("CANCEL", 130, 280, 100, 30, choice == 6 && !renderall, &direction);

        if (!renderall) {
            if (direction == RIGHT) { choice++; }
            else if (direction == LEFT) { choice--; }
            if (choice > 6) { choice = 0; }
            else if (choice < 0) { choice = 6; }
        }

        renderall = !renderall;

        if (confirm) {
            tm_time.tm_year = temp_year - 1900;
            ESP_LOGI("TIME", "Local time updated!");
            SaveTime(mktime(&tm_time));
            exit = true;
        }
        if (cancel) { exit = true; }
    }
}

/*Ringtone selector
 * not fully implemented yet
 * @param isMail true if the ringtone is for mail, false if it is for call
 */
void ringtoneSelector(bool isMail) {
#warning ringtoneSelector not implemented
    (void)isMail;
    // NFile* dir = VFS.open("/sd/AUDIO");
    // if (!dir) {
    //     ErrorWindow("NO /AUDIO");
    //     return;
    // }

    // File entry;

    // mOption opt[count];
    // int     iconIndex = -1;

    // NString *compareRingtone = isMail ? &currentMailRingtonePath : &currentRingtonePath;

    // count = 0;
    // while (entry = dir.openNextFile()) {
    //     if (!entry.isDirectory() && NString(entry.name()).endsWith(".SGAUDIO")) {
    //         pathes[count]    = entry.path();
    //         opt[count].label = entry.name();
    //         opt[count].label.replace(".SGAUDIO", "");

    //         // TODO DRAW NOTE ICON
    //         if (pathes[count].equals(*compareRingtone)) { iconIndex = count; }
    //         count++;
    //     }
    // }

    // int choice = 0;
    // while (choice != -1) {
    //     choice = listMenu(opt, count, false, LM_SETTINGS, "Set ringtone", true, choice);
    //     if (choice < 0) { return; }
    //     if (iconIndex >= 0) { opt[iconIndex].image = Image(); }
    //     // TODO DRAW NOTE ICON
    //     iconIndex        = choice;
    //     *compareRingtone = pathes[choice];
    // }
}
