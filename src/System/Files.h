#pragma once
#include "../GlobalVariables.h"
#include "../UI/ListMenu.h"
enum FS_Type { FS_NONE, FS_SD, FS_SPIFFS };

class NFile {
  public:
    File    file;
    FS_Type type;
    NFile(String path = "") {
        if (path.length()) { FileOpen(path); }
    }
    String GetPath() {
        String path;
        switch (type) {
        case FS_SD: return String("/sd") + file.path();
        case FS_SPIFFS: return String("/spiffs") + file.path();
        default: return "";
        }
    }
    void FileOpen(String path) {
        if (path.startsWith("/sd/")) {
            path = path.substring(3);
            file = SD.open(path);
            type = FS_SD;
        }
        else if (path.startsWith("/spiffs/")) {
            path = path.substring(7);
            file = SPIFFS.open(path);
            type = FS_SPIFFS;
        }
        else if (path.equals("/")) {
            file = SD.open(path);
            type = FS_SD;
        }
        else {
            file = File();
            type = FS_NONE;
        }
    }
    bool isValid() const { return file; };
    bool isDirectory() { return file.isDirectory(); }
    void remove() {
        switch (type) {
        case FS_SD: SD.remove(file.path()); break;
        case FS_SPIFFS: SPIFFS.remove(file.path()); break;
        case FS_NONE: break;
        }
    }
    void close() { file.close(); };
};

String fileBrowser(String path = "/", String format = "*", bool graphical = true);