#include "Settings.h"
#include <Screens/ImageViewer.h>
#include <System/LanguageSystem.h>
#include <algorithm>
#include <cctype>
const int lastImage = 42;
int       getDaysInMonth(int year, int month);

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
            NString filepath = fileBrowser("/", "|.nph|.NPH|");
            if (VFS.exists(filepath)) {
                NFile* resource = VFS.open(filepath);
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

std::map<std::string, image_data> wallpaperPreviewCache;

void drawSelectedFrame(uint16_t* buffer, int w, int h) {
    if (!buffer || w <= 0 || h <= 0) { return; }
    // draw red frame around the image
    uint16_t color = 0x00F8; // Red in RGB565
    for (int x = 0; x < w; x++) {
        buffer[x]               = color; // top red line
        buffer[(h - 1) * w + x] = color; // bottom red line
    }
    for (int y = 0; y < h; y++) {
        buffer[y * w]           = color; // left red line
        buffer[y * w + (w - 1)] = color; // right red line
    }
}

mOption wallpaperPreview(void* data) {
    wallpaper* wp = (wallpaper*)data;
    if (!wp->path.isEmpty()) {
        image_data idata = {}; // Initialize to zero

        auto it = wallpaperPreviewCache.find(wp->path);
        if (it != wallpaperPreviewCache.end()) { idata = it->second; }

        if (wallpaperPreviewCache.size() > 30) {
            auto first = wallpaperPreviewCache.begin();
            if (first->second.buffer) { free(first->second.buffer); }
            wallpaperPreviewCache.erase(first);
        }

        if (!idata.buffer) {
            image_data params = displayPNG(wp->path, 0, 0, true);
            if (params.errorReason || params.srcwidth <= 0 || params.srcheight <= 0) {
                ESP_LOGI("WP PREVIEW", "Error reading image info for preview: %s",
                         params.errorReason ? params.errorReason : "invalid dimensions");
                return mOption(wp->path.substring(wp->path.lastIndexOf('/') + 1), Image(), 0,
                               nullptr);
            }

            // 34x42 preview
            float ratio       = (float)params.srcwidth / (float)params.srcheight;
            int   previewMaxW = 34;
            int   previewMaxH = 42;
            int   w = 34, h = 42;
            if (params.srcheight > previewMaxH || params.srcwidth > previewMaxW) {
                if (ratio > 1) { // wider than taller
                    w = previewMaxW;
                    h = previewMaxW / ratio;
                }
                else { // taller than wider
                    h = previewMaxH;
                    w = previewMaxH * ratio;
                }
            }
            idata = displayPNG(wp->path, w, h, false);
            if (idata.errorReason || !idata.buffer) {
                ESP_LOGI("WP PREVIEW", "Error loading image for preview: %s",
                         idata.errorReason ? idata.errorReason : "no buffer");
                return mOption(wp->path.substring(wp->path.lastIndexOf('/') + 1), Image(), 0,
                               nullptr);
            }

            wallpaperPreviewCache[wp->path] = idata;
        }

        if (wp->path == currentWallpaper.path) {
            drawSelectedFrame(idata.buffer, idata.srcwidth, idata.srcheight);
        }

        return mOption(wp->path.substring(wp->path.lastIndexOf('/') + 1),
                       Image(idata.buffer, idata.srcwidth, idata.srcheight, idata.srcwidth,
                             idata.srcheight, true));
    }

    if (wp->id >= 0) {
        auto it = wallpaperPreviewCache.find(std::to_string(wp->id));
        if (it != wallpaperPreviewCache.end()) {
            image_data idata = it->second;
            if (wp->id == currentWallpaper.id) {
                drawSelectedFrame(idata.buffer, idata.srcwidth, idata.srcheight);
            }
            return mOption("Wallpaper " + NString(wp->id),
                           Image(idata.buffer, idata.srcwidth, idata.srcheight, idata.srcwidth,
                                 idata.srcheight, true));
        }

        int       previewMaxW = 34;
        int       previewMaxH = 42;
        ImageData idata       = res.GetImageDataByID(wp->id);
        if (idata.width <= 0 || idata.height <= 0) {
            return mOption("Wallpaper " + NString(wp->id), Image(), 0, nullptr);
        }
        float ratio = (float)idata.width / (float)idata.height;
        int   w     = 34;
        int   h     = 42;
        if (idata.height > previewMaxH || idata.width > previewMaxW) {
            if (ratio > 1) {
                w = previewMaxW;
                h = previewMaxW / ratio;
            }
            else {
                h = previewMaxH;
                w = previewMaxH * ratio;
            }
        }
        uint16_t* previewBuffer =
            resizeRGB565buffer(res.GetRGB565(idata).pointer, idata.width, idata.height, w, h);
        if (!previewBuffer) {
            return mOption("Wallpaper " + NString(wp->id), Image(), 0, nullptr);
        }
        if (wp->id == currentWallpaper.id) { drawSelectedFrame(previewBuffer, w, h); }
        image_data previewData                        = {w, h, nullptr, previewBuffer};
        wallpaperPreviewCache[std::to_string(wp->id)] = previewData;
        return mOption("Wallpaper " + NString(wp->id), Image(previewBuffer, 0, 0, w, h, 0), 0,
                       nullptr);
    }

    return mOption("", Image(), 0, nullptr);
}
wallpaper currentWallpaper;
void      drawWallpaper() {
    res.DrawImage(R_DEFAULT_WALLPAPER); // in case if wallpaper will fail
    if (currentWallpaper.id >= 0) { res.DrawImage(currentWallpaper.id); }
    else if (currentWallpaper.path.isEmpty()) { res.DrawImage(R_DEFAULT_WALLPAPER); }
    else if (VFS.exists(currentWallpaper.path)) {
        drawImageWithMode(currentWallpaper.path, currentWallpaper.mode, 0, 26);
    }
}
void changeWallpaper() {
    wallpaper     twp;
    const NString wallpaperModes[] = {
        getTranslation(TextKey::WALLPAPER_CENTERED), getTranslation(TextKey::WALLPAPER_TILED),
        getTranslation(TextKey::WALLPAPER_FILLED),   getTranslation(TextKey::WALLPAPER_STRETCHED),
        getTranslation(TextKey::WALLPAPER_FIT_HOR),  getTranslation(TextKey::WALLPAPER_FIT_VER)};
    const NString wallpaperConfirmOptions[] = {"Change mode",
                                               getTranslation(TextKey::CONFIRM_BUTTON),
                                               getTranslation(TextKey::CANCEL_BUTTON)};

    std::vector<std::string> wallpaperFiles = VFS.listDir(WALLPAPER_DIR);
    uint8_t                  resourceCount  = res.GetImageDataByID(R_DEFAULT_WALLPAPER).count;
    std::vector<mOption>     options;
    std::vector<wallpaper>   wallpaperEntries;
    wallpaperEntries.reserve(resourceCount + wallpaperFiles.size() + 1);
    std::vector<std::string> supportedFormats = {".png", ".jpg", ".jpeg", ".bmp",
                                                 ".tga", ".pic", ".gif"};
    for (int i = 0; i < resourceCount; i++) {
        wallpaperEntries.push_back({i, "", IMG_CENTERED});
        options.push_back(mOption("", Image(), 0, nullptr, 0, wallpaperPreview,
                                  (void*)&wallpaperEntries.back()));
    }
    for (const std::string& file : wallpaperFiles) {

        size_t dotPos = file.find_last_of('.');
        if (dotPos == std::string::npos) { continue; }
        std::string extension = file.substr(dotPos);
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
        if (std::find(supportedFormats.begin(), supportedFormats.end(), extension) !=
            supportedFormats.end()) {

            wallpaperEntries.push_back({-1, WALLPAPER_DIR + file, IMG_CENTERED});
            options.push_back(mOption("", Image(), 0, nullptr, 0, wallpaperPreview,
                                      (void*)&wallpaperEntries.back()));
        }
    }

    options.push_back(
        mOption(/*getTranslation(TextKey::LM_SET_WALLPAPER_FILE_BROWSER)*/ "more wallpapers"));
    NString path;
    size_t  selection = LISTMENU_NULL;
    while (selection != LISTMENU_EXIT) {
        res.DrawImage(R_MENU_BACKGROUND);
        res.DrawImage(R_SETTING_MENU_L_HEADER);
        selection = listMenu(options, options.size(), 1, LM_SETTINGS,
                             getTranslation(TextKey::LM_SET_CHNG_WALLPAPER), false, selection);
        if (selection < 0) { return; }
        if (selection >= 0 && selection < options.size() - 1) {
            if (!options[selection].getOptArgs) {
                ESP_LOGE("WP", "Options getOptArgs is null for selection %d", selection);
                continue;
            }

            if (((wallpaper*)options[selection].getOptArgs)->id >= 0) {
                currentWallpaper.id   = ((wallpaper*)options[selection].getOptArgs)->id;
                currentWallpaper.path = "";
                currentWallpaper.mode = IMG_CENTERED;
                preferences.begin("System");
                preferences.putString("wallpaper_path", currentWallpaper.path.c_str());
                preferences.putInt("wallpaper_mode", currentWallpaper.mode);
                preferences.putInt("wallpaper_id", currentWallpaper.id);
                preferences.end();
                continue;
            }
            twp = *(wallpaper*)options[selection].getOptArgs;
        }
        if (selection == options.size() - 1) {
            NString path = fileBrowser("/", "|.png|.jpg|.jpeg|.bmp|.tga|.pic|.gif|");
            if (path.isEmpty()) { return; }
            ESP_LOGI("E", "Path is %s", path.c_str());

            if (path.isEmpty()) { ESP_LOGI("E", "Path is empty! %s", path.c_str()); }
            twp = wallpaper{-1, path, IMG_CENTERED};
        }
        int wallpaperMode = IMG_CENTERED;
        if (twp.path.isEmpty()) { return; }
        while (true) {
            wallpaperMode = choiceMenu(wallpaperModes, ArraySize(wallpaperModes), true);
            drawImageWithMode(twp.path, (ImageMode)wallpaperMode, 0, 26);
            while (buttonsHelding() == -1);
            int confirm =
                choiceMenu(wallpaperConfirmOptions, ArraySize(wallpaperConfirmOptions), true);
            if (confirm == 1) {
                currentWallpaper.path = twp.path;
                currentWallpaper.id   = twp.id;
                currentWallpaper.mode = (ImageMode)wallpaperMode;
                preferences.begin("System");
                preferences.putString("wallpaper_path", currentWallpaper.path.c_str());
                preferences.putInt("wallpaper_mode", currentWallpaper.mode);
                preferences.putInt("wallpaper_id", currentWallpaper.id);
                preferences.end();
                return;
            }
            if (confirm == 2) { return; }

            if (wallpaperPreviewCache.size() > 0) {
                for (auto it = wallpaperPreviewCache.begin(); it != wallpaperPreviewCache.end();) {
                    if (it->second.buffer) { free(it->second.buffer); }
                    it = wallpaperPreviewCache.erase(it);
                }
            }
        }
    }
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
        int dayMax = getDaysInMonth(temp_year, tm_time.tm_mon);

        if (tm_time.tm_mday > dayMax) { tm_time.tm_mday = dayMax; }

        sNumberChange(57, 90, 25, 25, tm_time.tm_mday, 1, dayMax, choice == 0 && !renderall,
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

int getDaysInMonth(int year, int month) {
    static const int days[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && (year % 4 == 0)) { // Simplified leap year check (ignores century rules)
        return 29;
    }
    return days[month];
}
