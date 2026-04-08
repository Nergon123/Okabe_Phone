#include "init.h"
#include "System/ResourceSystem.h"
#ifdef PC
#include "Platform/Graphics/SDL2RenderTarget.h"
#else
#include "Platform/Graphics/TFTESPIRenderTarget.h"
#endif
#include <System/LanguageSystem.h>
// Function to initialize the storage
void storageInit() {
    hw->initStorage();
   // setLanguage("/spiffs/UA.ini");
    preferences.begin("System", false);
    resPath = preferences.getString("resPath", resPath.c_str());

    if (!VFS.exists(resPath)) {
        recovery(SplitString(getTranslation(TextKey::RECOVERY_FAIL_NO_RES)));
    }

    ESP_LOGI("RESOURCES", "LOADING RESOURCE FILE");
    progressBar(10, 100, 250);
    bootText(getTranslation(TextKey::BOOT_LOAD_RES));
    if (!res.Files) {
        NFile* Resource = VFS.open(resPath);
        res.Init(Resource, true);
    }
    if (!res.Files) { recovery(getTranslation(TextKey::RECOVERY_FAIL_FAIL_RES)); }

    currentWallpaperPath = preferences.getString("wallpaper", "");

    if (!VFS.exists(currentWallpaperPath)) {
        wallpaperIndex = preferences.getInt("wallpaperIndex", 0);
    }

    DIB_MS = preferences.getInt("DIB_MS", DIB_MS);
    preferences.end();
    progressBar(70, 100, 250);
}
