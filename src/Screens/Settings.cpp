#include "Settings.h"
#include <System/LanguageSystem.h>
const int lastImage = 42;
#include <Screens/ImageViewer.h>

void debugMenu() { InfoWindow("Nope.", IW_TITLE::INFO); }

void connectivityMenu() {
    NString options[] = {
        getTranslation(TextKey::LM_WIFI),
    };
    int selection = LISTMENU_NULL;
    while (selection != LISTMENU_EXIT) {
        res.DrawImage(R_MENU_BACKGROUND);
        res.DrawImage(R_SETTING_MENU_L_HEADER);
        selection = choiceMenu(options, ArraySize(options), false);
        switch (selection) {
        case 0: WiFiMenu(); break;
        }
    }
}

void inputLayouts() {
    std::vector<mOption> options;
    for (KeypadLayout keylayout : keypadLayouts) {
        options.push_back(
            mOption(keylayout.FullName, Image(R_FILE_MANAGER_ICONS),
                    keylayout.enabled ? LM_ICO_CHECK_CHECKED : LM_ICO_CHECK_UNCHECKED, nullptr,
                    keylayout.id));
    }

    int selection = LISTMENU_NULL;
    while (selection != LISTMENU_EXIT) {
        selection = listMenu(options, options.size(), false, LM_SETTINGS,
                             getTranslation(TextKey::LM_INP_LAYOUTS), false, selection);
        if (selection >= 0) {
            if (options[selection].data == keypadLayouts[selection].id) {
                keypadLayouts[selection].enabled = !keypadLayouts[selection].enabled;
                options[selection].icon_index    = keypadLayouts[selection].enabled
                                                       ? LM_ICO_CHECK_CHECKED
                                                       : LM_ICO_CHECK_UNCHECKED;
            }
        }
    }
}
void inputTimings() {
    NString confirmDelay =
        NString::format(getTranslation(TextKey::LM_INP_TIME_CONFIRM_DELAY).c_str(), DIB_MS);
    NString options[] = {confirmDelay};
    int     selection = LISTMENU_NULL;
    while (selection != LISTMENU_EXIT) {
        res.DrawImage(R_MENU_BACKGROUND);
        res.DrawImage(R_SETTING_MENU_L_HEADER);
        LM_RET_VALUE ret = listMenu(options, ArraySize(options), false, LM_SETTINGS,
                                    getTranslation(TextKey::LM_INP_TIMINGS), false, selection);
        selection        = ret.index;
        switch (selection) {
        case 0:
            if (ret.button == ANSWER) { DIB_MS -= 100; }
            else { DIB_MS += 100; }
            if (DIB_MS > 5000) { DIB_MS = 100; }
            if (DIB_MS < 100) { DIB_MS = 5000; }
            options[0] = NString::format(
                getTranslation(TextKey::LM_INP_TIME_CONFIRM_DELAY).c_str(), DIB_MS);
            break;
        }
    }
    preferences.begin("System");
    preferences.putInt("DIB_MS", DIB_MS);
    preferences.end();
}

void inputSettings() {
    NString options[] = {getTranslation(TextKey::LM_INP_LAYOUTS),
                         getTranslation(TextKey::LM_INP_TIMINGS)

    };
    int     selection = LISTMENU_NULL;
    while (selection != LISTMENU_EXIT) {
        res.DrawImage(R_MENU_BACKGROUND);
        res.DrawImage(R_SETTING_MENU_L_HEADER);
        selection = choiceMenu(options, ArraySize(options), false);
        switch (selection) {
        case 0: inputLayouts(); break;
        case 1: inputTimings(); break;
        }
    }
}

void languageSettings() {
    NString options[] = {"System English", getTranslation(TextKey::LM_SYS_LANG_CHOOSE)};
    int     selection = LISTMENU_NULL;
    while (selection != LISTMENU_EXIT) {
        res.DrawImage(R_MENU_BACKGROUND);
        res.DrawImage(R_SETTING_MENU_L_HEADER);
        selection = choiceMenu(options, ArraySize(options), false);
        switch (selection) {
        case 0:
            resetLanguage();
            preferences.begin("System");
            preferences.putString("Language", "");
            preferences.end();
            break;
        case 1:
            NString path = fileBrowser("/", ".ini");
            if (path.isEmpty()) { break; }
            NString err = setLanguage(path);
            if (!err.isEmpty()) { InfoWindow(err); }
            preferences.begin("System");
            preferences.putString("Language", path.c_str());
            preferences.end();
            break;
        }
    }
}

void systemSettings() {
    NString options[] = {
        getTranslation(TextKey::LM_SYS_DATE_TIME),
        getTranslation(TextKey::LM_SYS_INPUT),
        getTranslation(TextKey::LM_SYS_LANG),
    };
    int selection = LISTMENU_NULL;
    while (selection != LISTMENU_EXIT) {
        res.DrawImage(R_MENU_BACKGROUND);
        res.DrawImage(R_SETTING_MENU_L_HEADER);
        selection = choiceMenu(options, ArraySize(options), false);
        switch (selection) {
        case 0: setTime(); break;
        case 1: inputSettings(); break;
        case 2: languageSettings(); break;
        }
    }
}

void lookAndFeelSettings() {
    NString options[] = {
        getTranslation(TextKey::LM_LAF_CHANGE_THEME),
    };
    int selection = LISTMENU_NULL;
    while (selection != LISTMENU_EXIT) {
        res.DrawImage(R_MENU_BACKGROUND);
        res.DrawImage(R_SETTING_MENU_L_HEADER);
        selection = choiceMenu(options, ArraySize(options), false);
        switch (selection) {
        case 0:
            NString filepath = fileBrowser("/", ".nph");
            if (VFS.exists(filepath)) {
                NFile *resource = VFS.open(filepath);
                InfoWindow(getTranslation(TextKey::IW_APPLYING_THEME), IW_TITLE::INFO, false);
                res.Init(resource);
                res.CopyToRam();
                if (res.cache) { res.Files->close(); }
            }
            break;
        }
    }
}

void experimentalSettings() {
    NString options[] = {
        getTranslation(TextKey::LM_EXP_DEBUG_MENU),
    };
    int selection = LISTMENU_NULL;
    while (selection != LISTMENU_EXIT) {
        res.DrawImage(R_MENU_BACKGROUND);
        res.DrawImage(R_SETTING_MENU_L_HEADER);
        selection = choiceMenu(options, ArraySize(options), false);
        switch (selection) {
        case 0: debugMenu(); break;
        }
    }
}

void advancedSettings() {
    NString options[] = {
        getTranslation(TextKey::LM_A_SET_SYSTEM),
        getTranslation(TextKey::LM_A_SET_CONNECTIVITY),
        getTranslation(TextKey::LM_A_SET_LOOK_AND_FEEL),
        getTranslation(TextKey::LM_A_SET_EXPERIMENTAL),
    };
    int menuSelection = LISTMENU_NULL;
    while (menuSelection != LISTMENU_EXIT) {
        res.DrawImage(R_MENU_BACKGROUND);
        res.DrawImage(R_SETTING_MENU_L_HEADER);
        menuSelection = choiceMenu(options, ArraySize(options), false);
        switch (menuSelection) {
        case 0: systemSettings(); break;
        case 1: connectivityMenu(); break;
        case 2: lookAndFeelSettings(); break;
        case 3: experimentalSettings(); break;
        default: return;
        }
    }
}



void changeWallpaper() {
    const NString wallpaperModes[] = {
        getTranslation(TextKey::WALLPAPER_CENTERED), getTranslation(TextKey::WALLPAPER_TILED),
        getTranslation(TextKey::WALLPAPER_FILLED),   getTranslation(TextKey::WALLPAPER_STRETCHED),
        getTranslation(TextKey::WALLPAPER_FIT_HOR),  getTranslation(TextKey::WALLPAPER_FIT_VER)};
    int     wallpaperMode = IMG_CENTERED;
    NString path          = fileBrowser("/", "|.png|.jpg|.jpeg|.bmp|.tga|.pic|.gif|");
    ESP_LOGI("E", "Path is %s", path.c_str());

    if (path.isEmpty()) { ESP_LOGI("E", "Path is empty! %s", path.c_str()); }
    wallpaperMode = choiceMenu(wallpaperModes, ArraySize(wallpaperModes), true);
    drawImageWithMode(path, (ImageMode)wallpaperMode, 0, 26);

    wallpaperMode = IMG_CENTERED;
}

// Function to show the settings menu
// This function is called when the user wants to change settings
// It allows the user to change the wallpaper, ringtones, etc.
void settings() {
    NString settingsOptions[] = {getTranslation(TextKey::LM_SET_CHNG_WALLPAPER),
                                 getTranslation(TextKey::LM_SET_CALL_RINGTONE),
                                 getTranslation(TextKey::LM_SET_MAIL_RINGTONE),
                                 getTranslation(TextKey::LM_SET_ADVANCED_SET)};
    int     menuSelection     = LISTMENU_NULL;
    while (menuSelection != LISTMENU_EXIT) {
        res.DrawImage(R_MENU_BACKGROUND);
        res.DrawImage(R_SETTING_MENU_L_HEADER);
        menuSelection = choiceMenu(settingsOptions, ArraySize(settingsOptions), false);
        switch (menuSelection) {
        case 0: changeWallpaper(); break;
        case 1: ringtoneSelector(false); break;
        case 2: ringtoneSelector(true); break;
        case 3: advancedSettings(); break;
        default: break;
        }
    }
    currentScreen = SCREENS::MAINMENU;
}

// Set time screen
// This function is called when the user wants to set the time
// @param time Pointer to the time_t variable
void setTime() {
    res.DrawImage(R_LIST_MENU_BACKGROUND);
    drawHeader(getTranslation(TextKey::LM_SYS_DATE_TIME), LM_SETTINGS);
    changeFont(1);
    tft.setTextSize(1);
    tft.setTextColor(0);
    tft.setCursor(90, 77);
    tft.print(getTranslation(TextKey::MENU_DATE));
    tft.setCursor(57 + 28, 109);
    tft.print("/");
    tft.setCursor(93 + 28, 109);
    tft.print("/");
    tft.setCursor(90, 155);
    tft.print(getTranslation(TextKey::MENU_TIME));
    tft.setCursor(111, 187);
    tft.print(":");
    currentRenderTarget->present();
    time_t currentTime = hw->timeGet();
    tm     tm_time     = *gmtime(&currentTime);
    int    temp_year   = 1900 + tm_time.tm_year;
    // tm struct uses 0-11 for months, but we want to display 1-12...
    tm_time.tm_mon++;
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
        bool confirm = button(getTranslation(TextKey::CONFIRM_BUTTON), 10, 280, 100, 30,
                              choice == 5 && !renderall, &direction);
        bool cancel  = button(getTranslation(TextKey::CANCEL_BUTTON), 130, 280, 100, 30,
                              choice == 6 && !renderall, &direction);

        if (!renderall) {
            if (direction == RIGHT) { choice++; }
            else if (direction == LEFT) { choice--; }
            if (choice > 6) { choice = 0; }
            else if (choice < 0) { choice = 6; }
        }

        renderall = !renderall;

        if (confirm) {
            tm_time.tm_year = temp_year - 1900;
            tm_time.tm_mon--;
            ESP_LOGI("TIME", "Local time updated!");
            SaveTime(mktime(&tm_time));
            exit = true;
        }
        if (cancel) { exit = true; }
    }
}

/*Ringtone selector
 * not implemented yet
 * @param isMail true if the ringtone is for mail, false if it is for call
 */
void ringtoneSelector(bool isMail) {
#warning ringtoneSelector not implemented
    (void)isMail;
}
