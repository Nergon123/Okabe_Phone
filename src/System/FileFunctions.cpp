#include "FileFunctions.h"
const char* FTAG = "FileFunc";
#include <System/AudioPlayer.h>
#include <string>
#include <vector>
enum class FILE_FORMATS {
    TEXT,
    PICTURE,
    APP,
    AUDIO,
    THEME
};

bool LaunchApp(const NString& file) {}
bool PlayAudio(const NString& file) {
    AudioPlayer(file);
    return true;
}

bool NotSupported(const NString& file) {
    ESP_LOGI(FTAG, "File not supported");
    (void)file;
    InfoWindow("Cannot open file \n" + file);
    // maybe some context window
    return false;
}
bool ViewText(const NString& file) {
    ESP_LOGI(FTAG, "ViewText Not Implemented");
    (void)file;
    return false;
}
bool ViewPicture(const NString& file) {
    ImageViewer(file);
    ESP_LOGI(FTAG, "ViewPNG Not Implemented");
    (void)file;
    return false;
}
bool ApplyTheme(const NString& file) {
    if (!confirmation("Do you want to apply theme \n\"" + file + "\"?")) { return false; };
    tft.fillScreen(0);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(0, 30);
    tft.print("wait...");
    NFile* _file = VFS.open(file);
    if (_file) {
        res.Init(_file);
        res.CopyToRam();
        _file->close();
        tft.fillScreen(0);
        drawStatusBar(true);
        return true;
    }
    tft.fillScreen(0);
    drawStatusBar(true);

    return false;
}
struct Format_to_Func {
    FILE_FORMATS format;
    bool (*function)(const NString& file);
};
std::vector<std::pair<Format_to_Func, std::vector<std::string>>> file_formats = {
    {{FILE_FORMATS::TEXT, ViewText}, {".md", ".txt"}},
    {{FILE_FORMATS::PICTURE, ViewPicture},
     {".png", ".jpg", ".jpeg", ".bmp", ".tga", ".pic", ".gif"}},
    {{FILE_FORMATS::APP, LaunchApp}, {".lua", ".lpkg", ".zip"}},
    {{FILE_FORMATS::AUDIO, PlayAudio}, {".mp3"}},
    {{FILE_FORMATS::THEME,ApplyTheme},{".nph",".npz"}}};

bool FileOpen(const NString& file) {
    for (auto format : file_formats) {
        for (std::string dot_format : format.second) {
            if (file.toLowerCase().endsWith(dot_format)) { return format.first.function(file); }
        }
    }
    if (file.isEmpty()) { return true; }
    return NotSupported(file);
}