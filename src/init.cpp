#include "init.h"
#include "System/ResourceSystem.h"
#ifdef PC
#include "Platform/Graphics/SDL2RenderTarget.h"
#else
#include "Platform/Graphics/TFTESPIRenderTarget.h"
#endif

// Function to initialize the storage
void storageInit() {
    hw->initStorage();

    preferences.begin("System", false);
    resPath = preferences.getString("resPath", resPath.c_str());

    if (!VFS.exists(resPath)) {
        recovery(SplitString("Seems that you flashed your device wrongly.Refer to the "
                             "instructions for more information."));
    }

    ESP_LOGI("RESOURCES", "LOADING RESOURCE FILE");
    progressBar(10, 100, 250);
    bootText("Loading resource file...");
    if (!res.Files) {
        NFile* Resource = VFS.open(resPath);
        res.Init(Resource, true);
    }
    if (!res.Files) { recovery("There was an error when loading resource file."); }

    currentWallpaperPath = preferences.getString("wallpaper", "");

    if (!VFS.exists(currentWallpaperPath)) {
        wallpaperIndex = preferences.getInt("wallpaperIndex", 0);
    }

    DIB_MS = preferences.getInt("DIB_MS", DIB_MS);
    preferences.end();
    progressBar(70, 100, 250);
}
