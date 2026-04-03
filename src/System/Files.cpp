#include "Files.h"
#include <System/LanguageSystem.h>

struct iconFormat {
    uint8_t     icon;
    const char* format;
};

const iconFormat fileFormats[] = {
    {LM_ICO_IMAGE, "|.png|.jpg|.jpeg|.bmp|.tga|.pic|.gif|"},
    {LM_ICO_AUDIO, "|.wav|.mp3|.m4a|.flac|"},
    {LM_ICO_THEME, "|.nph|"},
    {LM_ICO_TEXT, "|.md|.txt|"},
    {0, ""},
};

uint8_t getIconByFormat(NString name) {
    NString format = name.substring(name.lastIndexOf('.'), name.length() - 1);
    for (size_t i = 0; i < sizeof(fileFormats) / sizeof(fileFormats[0]); i++) {
        if (format.indexOf("|" + NString(fileFormats[i].format) + "|") != -1) {
            return fileFormats[i].icon;
        }
        return 0;
    }
    return 0;
}
NString normalize(NString p) {
    // Prevent deleting the root slash
    while (p.endsWith("/") && p != "/") { p.remove(p.length() - 1); }
    return p;
}

// ## File browser
//
// This function allows the user to browse files on the SD card
//
// @param dir: Directory to browse
// @param format: File format to filter (e.g., ".txt") or |.txt|.png| for multiple files
// @param graphical: If true, use graphical interface
// @return The path of the selected file or empty string if cancelled
NString fileBrowser(NString path, NString format, bool graphical, NString title, bool saveMode,
                    NString saveFilename) {
    (void)title;
    while (true) {
        std::vector<mOption> entries;
        Image                iconSet = Image(R_FILE_MANAGER_ICONS);

        if (path != "/") { entries.push_back(mOption(getTranslation(TextKey::FILEMANAGER_PREV_FOLDER), iconSet, LM_ICO_FOLDER)); }

        if (saveMode) { entries.push_back(mOption(getTranslation(TextKey::FILEMANAGER_SAVE_HERE), iconSet, LM_ICO_FILE)); }

        std::vector<std::string> files = VFS.listDir(path);

        for (const std::string& name : files) {
            NString full = path;
            if (!full.endsWith("/")) { full += "/"; }
            full += name;

            IFile* f = VFS.open(full);

            // Special cases
            if (full == "/spiffs") {
                entries.push_back(mOption(name + "/", iconSet, LM_ICO_INTERNAL_STORAGE));
            }
            else if (full == "/sd") {
                entries.push_back(mOption(name + "/", iconSet, LM_ICO_SDCARD));
            }
            else if (f && f->isDirectory()) {
                entries.push_back(mOption(name + "/", iconSet, LM_ICO_FOLDER));
            }
            else {
                bool show = false;
                if (format == "*" || format.isEmpty()) { show = true; }
                else if (NString(name).endsWith(format)) { show = true; }
                else if (format.indexOf("|" + name.substr(name.find_last_of('.')) + "|") != -1) {
                    show = true;
                }

                if (show) { entries.push_back(mOption(name, iconSet, getIconByFormat(name))); }
            }

            if (f) {
                f->close();
                delete f;
            }
        }

        int selection = graphical
                            ? listMenu(entries, entries.size(), false, LM_SETTINGS, path).index
                            : listMenuNonGraphical(entries, entries.size(), path);

        if (selection < 0) {
            return NString(); // return empty string to indicate cancel
        }

        if (selection == 0 && path != "/") {
            path       = normalize(path);
            size_t pos = path.lastIndexOf('/');
            if (pos == 0) { path = "/"; }
            else { path = path.substring(0, pos); }
            continue;
        }
        if (saveMode && selection == 1) {
            if (saveFilename.isEmpty()) { return path; }
            if (!path.endsWith("/")) { path += "/"; }
            return path + saveFilename;
        }
        int     idx    = selection;
        NString chosen = entries[idx].label;

        if (!path.endsWith("/")) { path += "/"; }
        path += chosen;

        if (path.endsWith("/")) {
            if (!VFS.exists(path)) { return path; }
            IFile* ftest = VFS.open(path);
            if (!ftest || !ftest->isDirectory()) {
                delete ftest;
                return path;
            }
            delete ftest;
            continue;
        }
        return path;
    }
}

void FileManager() {}