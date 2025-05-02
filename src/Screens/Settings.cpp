#include "Settings.h"

const int lastImage = 42;
// Function to show the settings menu
// This function is called when the user wants to change settings
// It allows the user to change the wallpaper, ringtones, etc.
void settings() {

    drawFromSd(0, 26, SUBMENU_BACKGROUND); 
    drawFromSd(0, 26, SETTINGS_HEADER_IMAGE);
    String settingsOptions[] = {
        "Change Wallpaper",
        "Set call ringtone",
        "Set mail ringtone",
    };
    int menuSelection     = choiceMenu(settingsOptions, 3, false);
    int pictureIndex   = -1;
    int confPictureIndex = -2;

    while (true) {
        switch (menuSelection) {
        case -1:
            currentScreen = SCREENS::MAINMENU;
            return;
            break;
        case 0:
            pictureIndex = gallery();

            if (pictureIndex == -1) {
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
        Serial.println(pictureIndex);
        String galch[] = {"Preview", "Confirm"};
        if (pictureIndex != -1 && pictureIndex != lastImage) {

            confPictureIndex = choiceMenu(galch, 2, true);
            switch (confPictureIndex) {
            case 0:
                drawFromSd(WALLPAPER_IMAGES_BASE.address + WALLPAPER_IMAGES_MULTIPLIER * pictureIndex, 0, 26, WALLPAPER_IMAGES_BASE.w, WALLPAPER_IMAGES_BASE.h, "/" + resPath, 0, 0);
                while (buttonsHelding() != BACK)
                    ;
                break;
            case 1:
                preferences.begin("settings", false);
                preferences.putUInt("wallpaperIndex", pictureIndex);
                preferences.putString("wallpaper", "/null");
                preferences.end();
                wallpaperIndex = pictureIndex;
                free(wallpaper);
                wallpaper            = nullptr;
                currentWallpaperPath = "/null";
                currentScreen        = SCREENS::MAINSCREEN;
                return;
                break;
            default:

                break;
            }
        } else if (pictureIndex == lastImage) {
            String path = fileBrowser(SD.open("/"), ".png");
            confPictureIndex       = choiceMenu(galch, 2, true);
            switch (confPictureIndex) {
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


// Function to show the gallery menu
// This function is called when the user wants to change the wallpaper
int gallery() {
    if (!SD.exists(resPath))
        return lastImage;
    mOption wallpaperOptions[] = {
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
    for (int i = 0; i < ArraySize(wallpaperOptions) - 1; i++) {
        wallpaperOptions[i].icon = SDImage(WALLPAPER_ICONS_BASE.address + (WALLPAPER_ICONS_MULTIPLIER * i), WALLPAPER_ICONS_BASE.w, WALLPAPER_ICONS_BASE.h);
    }
    return listMenu(wallpaperOptions, ArraySize(wallpaperOptions), true, 2, "Change wallpaper");
}

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
void ringtoneSelector(bool isMail)
 {
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
