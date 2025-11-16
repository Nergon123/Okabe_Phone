#include "init.h"
#include "System/ResourceSystem.h"
#ifdef PC
#include "Platform/Graphics/SDL2RenderTarget.h"
#else
#include "Platform/Graphics/TFTESPIRenderTarget.h"
#endif


// Function to initialize the storage
void storageInit() {

#ifndef PC
    SPIFFS.begin();
    IFileSystem* spiffs = new Esp32FileSystem(&SPIFFS, FS_INTERNAL);
    IFileSystem* sdcard = new Esp32FileSystem(&SD, FS_EXTERNAL);
#else
    IFileSystem* spiffs = new StdFileSystem("spiffs/", FS_INTERNAL);
    IFileSystem* sdcard = new StdFileSystem("sd/", FS_EXTERNAL);
#endif

    sdcard->begin();
    VFS.mount("/sd", sdcard);
    spiffs->begin();
    VFS.mount("/spiffs", spiffs);
    ESP_LOGI("SPIFFS", "SPIFFS started");

    preferences.begin("settings", false);
    resPath = preferences.getString("resPath", resPath.c_str());

    if (!VFS.exists(resPath)) {
        recovery(SplitString("Seems that you flashed your device wrongly.Refer to the "
                             "instructions for more information."));
    }

    ESP_LOGI("RESOURCES", "LOADING RESOURCE FILE");
    progressBar(10, 100, 250);
    bootText("Loading resource file...");
    if (!res.Files[RES_MAIN]) {
        NFile* Resource = VFS.open(resPath);
        res.Init(Resource);
    }
    if (!res.Files[RES_MAIN]) { recovery("There was an error when loading resource file."); }

    currentWallpaperPath = preferences.getString("wallpaper", "");

    if (!VFS.exists(currentWallpaperPath)) {
        wallpaperIndex = preferences.getInt("wallpaperIndex", 0);
    }

    preferences.end();
    progressBar(70, 100, 250);
}
