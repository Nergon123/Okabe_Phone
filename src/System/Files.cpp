#include "Files.h"

/*
Icons TODO:
File
Folder
SD
SPIFFS
IMAGE
AUDIO
NPH
TXT
*/

FS_Type currentFS;
struct iconFormat {
    uint8_t     icon;
    const char *format;
};

const iconFormat fileFormats[] PROGMEM = {
    {4, ".png"},              // image icon
    {5, ".wav"}, {5, ".mp3"}, // audio icon
    {6, ".nph"},              // theme icon
    {7, ".txt"},              // doc  icon
    {0, ""},                  // default icon
};

uint8_t getIconByFormat(String &name) {
    for (size_t i = 0; i < sizeof(fileFormats) / sizeof(fileFormats[0]); i++) {
        if (name.endsWith(fileFormats[i].format)) { return fileFormats[i].icon; }
    }
    return 0;
}

NFile FileOpen(String path) {
    NFile nf;

    if (path.startsWith("/sd/")) {
        path    = path.substring(3);
        nf.file = SD.open(path);
        nf.type = FS_SD;
    }
    else if (path.startsWith("/spiffs/")) {
        path    = path.substring(7);
        nf.file = SPIFFS.open(path);
        nf.type = FS_SPIFFS;
    }
    else if (path.length() > 3) {
        nf.file = SD.open(path);
        nf.type = FS_SD;
    }
    else {
        nf.file = File();
        nf.type = FS_NONE;
    }

    return nf;
}

// ## File browser
//
// This function allows the user to browse files on the SD card
//
// @param dir: Directory to browse
// @param format: File format to filter (e.g., ".txt")
// @param graphical: If true, use graphical interface
// @return The path of the selected file or empty string if cancelled
String fileBrowser(String path, String format, bool graphical, String title) {
    NFile dir(path);

    // Mount both
    bool sdAvailable     = SD.begin(chipSelect, SPI);
    bool spiffsAvailable = SPIFFS.begin(false);
    // Choose filesystem
    mOption opts[2];
    int     count = 0;
    if (sdAvailable && SD.open("/")) {
        opts[count++] = {"MicroSD card", Image(R_FILE_MANAGER_ICONS), 2};
    }
    if (spiffsAvailable && SPIFFS.open("/")) {
        opts[count++] = {"Integrated memory", Image(R_FILE_MANAGER_ICONS), 3};
    }

    if (count == 0) { return ""; }

    int ch = graphical ? listMenu(opts, count, false, 2, title)
                       : listMenuNonGraphical(opts, count, title);

    if (ch < 0) { return ""; }

    if (opts[ch].label == "MicroSD card") {
        dir.file = SD.open("/");
        dir.type = FS_SD;
    }
    else {
        dir.file = SPIFFS.open("/");
        dir.type = FS_SPIFFS;
    }

    if (!dir.file) {
        ESP_LOGE("Browser", "Failed to open root directory!");
        return "";
    }

    // File browsing loop
    String   currentPath = dir.GetPath();
    mOption *options     = new mOption[64];
    int      choice      = -3;

    do {
        int i = 0;
        dir.file.rewindDirectory();
        File entry = dir.file.openNextFile();

        while (entry) {
            String name = entry.name();
            //  ESP_LOGI("TAG","Name: %s | isDir: %d\n", entry.path(), entry.isDirectory());
            if (entry.isDirectory()) {
                options[i++] = {name + "/", Image(R_FILE_MANAGER_ICONS), 1};
                ESP_LOGI("DIR", "DIR %s", name);
            }
            else {
                String fileLow = name;
                fileLow.toLowerCase();
                if (format == "*" || fileLow.endsWith(format)) {
                    options[i++] = {name, Image(R_FILE_MANAGER_ICONS), getIconByFormat(fileLow)};
                }
            }
            entry = dir.file.openNextFile();
        }

        choice = graphical ? listMenu(options, i, false, 2, title)
                           : listMenuNonGraphical(options, i, title);

        if (choice >= 0 && options[choice].label.endsWith("/")) {
            String folder = options[choice].label;
            folder.remove(folder.lastIndexOf("/"));
            currentPath += "/" + folder;
        }
        else if (choice >= 0) {
            String selected = options[choice].label;
            delete[] options;
            dir.close();
            return currentPath + "/" + selected;
        }
    }
    while (choice >= 0);

    delete[] options;
    dir.close();
    return (choice == -2) ? "--" : "";
}

void FileManager() {
    // TODO:
    //  - Deleting
    //  - Copying
    //  - moving
    //  - Opening by File extension
    String path = "/";
    bool   exit = false;
    while (!exit) {
        path = fileBrowser(path, "*", false);

        String choices[] = {getActionName(path.substring(path.lastIndexOf("/"))),

                            "Delete", "Copy", "Move", "Exit"};
        String secondPath;
        int    ch = choiceMenu(choices, ArraySize(choices), true);
        switch (ch) {
        case 0: FileOpen(path); break;
        case 1: NFile(path).remove(); break;
        case 2:
            secondPath = fileBrowser(path, "*", 1, "Copy to...");
            NFile(path).copy(NFile());
            break;
        case 3:
            secondPath = fileBrowser(path, "*", 1, "Move to...");
            NFile(path).move(NFile());
            break;
        case 4: return;
        }
    }
}