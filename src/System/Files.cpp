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

struct iconFormat {
    uint8_t     icon;
    const char *format;
};

#ifdef PC
const iconFormat fileFormats[] = {
#else
const iconFormat fileFormats[] PROGMEM = {
#endif
    {4, ".png"},              // image icon
    {5, ".wav"}, {5, ".mp3"}, // audio icon
    {6, ".nph"},              // theme icon
    {7, ".txt"},              // doc  icon
    {0, ""},                  // default icon
};

uint8_t getIconByFormat(NString &name) {
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
NString fileBrowser(NString path, NString format, bool graphical, NString title) {
    return "NULL";
}

void FileManager() {

}