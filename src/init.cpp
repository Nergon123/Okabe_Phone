#include "init.h"
#include "System/ResourceSystem.h"
#ifdef PC
#include "Platform/Graphics/SDL2RenderTarget.h"
#else
#include "Platform/Graphics/TFTESPIRenderTarget.h"
#endif
#include <System/LanguageSystem.h>
#include <System/properties.h>
// Function to initialize the storage
void storageInit() {
    hw->initStorage();

    char prop_val[PROPERTY_VALUE_MAX];
    property_get(PROPERTIES_KEY_RESPATH, prop_val, resPath.c_str());
    resPath = NString(prop_val);

    if (!VFS.exists(resPath)) {
        recovery(SplitString(getTranslation(TextKey::RECOVERY_FAIL_NO_RES)));
    }

    ESP_LOGI("RESOURCES", "LOADING RESOURCE FILE %s", resPath.c_str());
    progressBar(10, 100, 250);
    bootText(getTranslation(TextKey::BOOT_LOAD_RES));
    if (!res.Files) {
        NFile* Resource = VFS.open(resPath);
        res.Init(Resource, true);
        ESP_LOGI("RESOURCES","Initialized file %s",Resource->path().c_str());
    }

    if (!res.Files) { recovery(getTranslation(TextKey::RECOVERY_FAIL_FAIL_RES)); }

    property_get(PROPERTIES_KEY_WALLPAPER, prop_val, "");
    currentWallpaperPath = NString(prop_val);

    if (!VFS.exists(currentWallpaperPath)) {
        wallpaperIndex = property_get_long(PROPERTIES_KEY_WALLPAPER_INDEX, 0);
    }

    DIB_MS = property_get_long(PROPERTIES_KEY_DIB_MS, DIB_MS);
    progressBar(70, 100, 250);
}
