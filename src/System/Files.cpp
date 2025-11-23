#include "Files.h"

struct iconFormat {
    uint8_t     icon;
    const char *format;
};

const iconFormat fileFormats[] = {
    {LM_ICO_IMAGE, ".png"}, {LM_ICO_AUDIO, ".wav"}, {LM_ICO_AUDIO, ".flac"},
    {LM_ICO_THEME, ".nph"}, {LM_ICO_TEXT, ".txt"},  {0, ""},
};

uint8_t getIconByFormat(NString name) {
    for (size_t i = 0; i < sizeof(fileFormats) / sizeof(fileFormats[0]); i++) {
        if (name.endsWith(fileFormats[i].format)) { return fileFormats[i].icon; }
    }
    return 0;
}

// ## File browser
//
// This function allows the user to browse files on the SD card
//
// @param dir: Directory to browse
// @param format: File format to filter (e.g., ".txt")
// @param graphical: If true, use graphical interface
// @return The path of the selected file or empty string if cancelled
NString fileBrowser(NString path, NString format, bool graphical, NString title, bool saveFile) {
#warning fileBrowser probably needs reimplementing/fixing/testing
    IFile               *begin = VFS.open(path);
    std::vector<mOption> dirEntries;
    Image                img  = Image(R_FILE_MANAGER_ICONS);
    mOption              back = mOption("..", img, LM_ICO_FOLDER);
    mOption              saveHere =
        mOption("Save here", img, LM_ICO_FILE); // TODO: maybe replace with floppy icon
    while (true) {
        title = path;
        if (path != "/") { dirEntries.push_back(back); }
        if (saveFile) { dirEntries.push_back(saveHere); }
        std::vector<std::string> files = VFS.listDir(path);
        for (std::string file : files) {
            std::string t_path = path + file;
            begin              = VFS.open(t_path);
            if (t_path.compare("/spiffs") == 0) {
                dirEntries.push_back(mOption(file + "/", img, LM_ICO_INTERNAL_STORAGE));
                continue;
            }
            if (t_path.compare("/sd") == 0) {
                dirEntries.push_back(mOption(file + "/", img, LM_ICO_SDCARD));
                continue;
            }
            if (begin->isDirectory()) {
                dirEntries.push_back(mOption(file + "/", img, LM_ICO_FOLDER));
                continue;
            }
            if ((format == "*" || format.isEmpty()) ||
                (format.isEmpty() && NString(file).endsWith(format))) {
                dirEntries.push_back(mOption(file, img, getIconByFormat(file)));
            }
        }
        files.clear();
        int selection = graphical
                            ? listMenu(dirEntries, dirEntries.size(), false, LM_SETTINGS, title)
                            : listMenuNonGraphical(dirEntries, dirEntries.size(), title);
        if (selection >= (saveFile ? 2 : 1) || path == "/") {
            bool root = path == "/";

            path += dirEntries[selection].label;
            if (root) {
                dirEntries.clear();
                continue;
            }
        }
        else if (saveFile && selection == 1) { return path + "/" + saveFile; }
        else if (selection == 0 && path != "/") {
            path = path.substring(path.lastIndexOf('/', 0), path.length() - 1);
        }
        else if (selection < 0) { return (char)selection + "\0"; }
        else {}
        if (begin) { free(begin); }
        if (VFS.exists(path)) {
            begin = VFS.open(path);
            if (!begin->isDirectory()) { return path; }
        }
        else { return path; }
        dirEntries.clear();
    }
}

void FileManager() {}