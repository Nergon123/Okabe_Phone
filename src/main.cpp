#include "Screens/Main.h"
#include "BuiltinPackages.h"
#include "GlobalVariables.h"
#include "Platform/Graphics/Fonts/FontLoader.h"
#include "Platform/Hardware/Hardware.h"
#include "Platform/Hardware/Profiles/ESP32.h"
#include "Platform/Hardware/Profiles/Linux.h"
#include "System/ResourceSystem.h"
#include "System/properties.h"
#include "System/Tasks.h"
#include "System/Time.h"
#include "init.h"
#include <Platform/Audio/AudioGen.h>
#include <System/LanguageSystem.h>
#include <System/AudioPlayer.h>
#ifdef IDF_VER
TaskHandle_t *TaskLoop_Handle;

void TaskLoop(void *) {
    setup();
    while (true) {
        loop();
        vTaskDelay(1);
    }
}
extern "C" void app_main(void) {

    initArduino();
    xTaskCreate(TaskLoop, "TaskLoop", 20 * 1024, NULL, 2, TaskLoop_Handle);
    vTaskDelete(NULL);
}
#endif

int start() {
#ifdef PC
    hw = new DEV_LINUX();
#elif defined(ESP32)
    hw = new DEV_ESP32();
#endif

    SetUpTime();
    // hardwareInit();
    hw->init();
    currentRenderTarget = hw->GetScreen();
    ESP_LOGI("INIT", "Main render target %p", currentRenderTarget);
    if (!currentRenderTarget) {
        ESP_LOGE("INIT", "Error occurred when initializing screen (GetScreen returned nullptr)");
        return 2;
    }
    currentRenderTarget->init();
    tft.setRenderTarget(currentRenderTarget);
    hw->postScreenInit();
    if (hw->isCharging()) { offlineCharging(); }
    currentRenderTarget->setUseBuffer(false);
    tft.fillScreen(0x0000);
    tft.setTextFont(1);
    tft.setCursor(0, 0);
    progressBar(0, 100, 250);

    // Chance to change resource file to custom one
    storageInit();
    if (buttonsHelding(false) == '*') { recovery(getTranslation(TextKey::RECOVERY_MANUAL)); }

    res.CopyToRam(true);
    if (res.cache) { res.Files->close(); }
    res.DrawImage(R_BOOT_LOGO);
    bootText(getTranslation(TextKey::BOOT_INIT_RTOS));
    initBackgroundTasks();

    ESP_LOGI("DEVICE",
             FIRMVER "\n\n Phone firmware written by Nergon123 and contributors\n\n "
                     "Resources located in %s\n",
             resPath.c_str());

    ESP_LOGI("DEVICE", "%s", hw->getDeviceName());

    // register static packages
    pm.registerPackage(&SPKG(PkgMgr));

    // register package from fs
    PaStor.init();
    char prop_val[PROPERTY_VALUE_MAX];
    property_get(PROPERTIES_KEY_LANGUAGE, prop_val, "");
    setLanguage(prop_val);
    property_get(PROPERTIES_KEY_WALLPAPER_PATH, prop_val, "");
    currentWallpaper.path = NString(prop_val);
    currentWallpaper.mode = (ImageMode)property_get_long(PROPERTIES_KEY_WALLPAPER_MODE,IMG_CENTERED);
    currentWallpaper.id = property_get_long(PROPERTIES_KEY_WALLPAPER_ID,0);
    progressBar(100, 100, 250);
    if (buttonsHelding(false) == '#') { AT_test(); }
    

    currentRenderTarget->setUseBuffer(true);
    millSleep = hw->millis();
    return 0;
}

void setup() { start(); }
// Function to handle the main loop
void loop() { screens(); }

#ifdef PC
int main(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--scale") == 0) {
            if (argv[i + 1]) {
                SDLScale = atoi(argv[i + 1]);
                if (SDLScale < 1 || SDLScale > 9) {
                    printf("\n--scale value should be in range from 1 to 9\n");
                    return -1;
                }

                SDLScale = SDLScale > 0 ? SDLScale : 1;
                ESP_LOGI("MAIN", "Set SDL scale to %d", SDLScale);
                i++;
            }
            else {
                printf("--scale needs value from 1 to 9");
                return -1;
            }
        }
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Phone " FIRMVER "\n" REPOSITORY_LINK "\n"
                   "\n\t--help, -h         Show this screen"
                   "\n\t--scale n, -s n    Multiply window size by n times"
                   "\n\n");
            return 0;
        }
    }
    int err = start();
    if (err != 0) { return err; }
    while (true) { loop(); }
    return 0;
}
#endif
