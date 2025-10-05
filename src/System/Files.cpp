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

// ## File browser
//
// This function allows the user to browse files on the SD card
//
// @param dir: Directory to browse
// @param format: File format to filter (e.g., ".txt")
// @param graphical: If true, use graphical interface
// @return The path of the selected file or "/null" if cancelled
String fileBrowser(File dir, String format, bool graphical) {

    String currentPath = dir.path();
    File   file        = dir.openNextFile();
    dir.rewindDirectory();
    mOption *options = new mOption[64];
    int      choice  = -3;
    int      i       = 0;
    format.toLowerCase();
    do {
        i   = 0;
        dir = SD.open(currentPath);
        dir.rewindDirectory();
        file = dir.openNextFile();
        Serial.println(currentPath);
        while (file) {
            if (strlen(file.name()) != 0) {

                Serial.println();
                Serial.print(file.name());
                if (file.isDirectory()) {
                    Serial.print("/");

                    options[i].image       = Image(R_FILE_MANAGER_ICONS);
                    options[i].icon_index = 1;
                    options[i].label      = file.name() + String("/");
                }
                else {
                    String fileLow = String(file.name());
                    fileLow.toLowerCase();
                    if (format == "*" || fileLow.endsWith(format)) {
                        options[i].image       = Image(R_FILE_MANAGER_ICONS);
                        options[i].icon_index = 0; // I absolutely not sure about this...
                        options[i].label      = file.name();
                    }
                    else { i--; }
                }
                i++;
                file = dir.openNextFile();
            }
        }
        
        if (graphical) { choice = listMenu(options, i, false, 2, "FILE MANAGER"); }
        else { choice = listMenuNonGraphical(options, i, "FILE MANAGER"); }

        if (options[choice].label.endsWith("/")) {
            options[choice].label.remove(options[choice].label.lastIndexOf("/"));
            currentPath += "/" + options[choice].label;
        }
        else {
            currentPath += "/" + options[choice].label;
            return currentPath;
        }
        dir.close();
    } while (choice >= 0);
    if (choice == -2) { return "--"; }
    return "/null"; //Weird stuff that I was writing
    file.close();
    dir.close();
}
