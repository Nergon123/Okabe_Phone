#include "Files.h"


// ## File browser
//
// This function allows the user to browse files on the SD card
//
// @param dir: Directory to browse
// @param format: File format to filter (e.g., ".txt")
// @param graphical: If true, use graphical interface
// @return The path of the selected file or "/null" if cancelled
String fileBrowser(File dir, String format, bool graphical) {
    SDImage folderIcon  = SDImage(0x663983, 18, 18, 0, true);
    SDImage fileIcon    = SDImage(0x663984 + (18 * 18 * 2), 18, 18, 0, true);
    String  currentPath = dir.path();
    File    file        = dir.openNextFile();
    dir.rewindDirectory();
    mOption *options = new mOption[64];
    int      choice  = -2;
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

                    options[i].icon  = folderIcon;
                    options[i].label = file.name() + String("/");
                } else {
                    String fileLow = String(file.name());
                    fileLow.toLowerCase();
                    if (format == "*" || fileLow.endsWith(format)) {
                        options[i].icon  = fileIcon;
                        options[i].label = file.name();
                    } else
                        i--;
                }
                i++;
                file = dir.openNextFile();
            }
        }
        if (graphical)
            choice = listMenu(options, i, false, 2, "FILE MANAGER");
        else
            choice = listMenuNonGraphical(options, i, "FILE MANAGER");

        if (options[choice].label.endsWith("/")) {
            options[choice].label.remove(options[choice].label.lastIndexOf("/"));
            currentPath += "/" + options[choice].label;
        } else {
            currentPath += "/" + options[choice].label;
            return currentPath;
        }
        dir.close();
    } while (choice != -1);
    return "/null";
    file.close();
    dir.close();
}
