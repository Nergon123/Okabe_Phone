#pragma once
#define HOSTNAME        "okabe_phone"
#define PRODUCTNAME     "OkabePhone"
#define FIRMVER         "0.2.0 ALPHA"
#define REPOSITORY_LINK "https://github.com/Nergon123/Okabe_Phone"

#define LISTMENU_CALL    -4
#define LISTMENU_NULL    -3
#define LISTMENU_OPTIONS -2
#define LISTMENU_EXIT    -1
#define LISTMENU_TOHOME  -5
#define LISTMENU_BUFFER  true

#ifdef PC

#define psramFound()             1
#define ps_malloc(sz)            malloc(sz)
#define ps_realloc(ptr, newsize) realloc(ptr, newsize)

#define PROGMEM

#ifndef ESP_LOGG
#define ESP_LOGG(level, tag, format, ...)                                                         \
    ::printf("[%s:%d][%s][%s] " format "\n", __FILE__, __LINE__, level, tag, ##__VA_ARGS__)
#endif
#ifndef ESP_LOGI
#define ESP_LOGI(tag, format, ...) ESP_LOGG("INFO", tag, format, ##__VA_ARGS__)
#endif
#ifndef ESP_LOGW
#define ESP_LOGW(tag, format, ...) ESP_LOGG("WARN", tag, format, ##__VA_ARGS__)
#endif
#ifndef ESP_LOGE
#define ESP_LOGE(tag, format, ...) ESP_LOGG("ERROR", tag, format, ##__VA_ARGS__)
#endif
#ifndef ESP_LOGD
#define ESP_LOGD(tag, format, ...) ESP_LOGG("DEBUG", tag, format, ##__VA_ARGS__)
#endif
#ifndef ESP_LOGV
#define ESP_LOGV(tag, format, ...) ESP_LOGG("VERBOSE", tag, format, ##__VA_ARGS__)
#endif

#else
#include <esp_log.h>
#endif

#define NI_delay(old_millis, ms) (hw->millis() - old_millis < ms)

#ifndef ulong
typedef unsigned long ulong;
#endif

// delay between SIM card checks
#define DBC_MS 5000

#define USER_DIR "/sd/"
#define SYSTEM_DIR "/spiffs/"
#define WALLPAPER_DIR "/sd/Wallpapers/"

#define SimSerial Serial1

#define lines_to_draw_wo_psram 3

#ifdef OKABE_PHONE_HW_0
#define SIM_INT_PIN 34
#define HAVE_AMPL
#else
// #define SIM_INT_PIN 36
#define VIBRO_PIN 34
#endif

#define AMPL_BCLK  12
#define AMPL_LRCLK 25
#define AMPL_DIN   19

// INPUT OF inputText() WINDOW
#define INPUT_LOCATION_Y 240

// CURSOR WIDTH (PIXELS)
#define CWIDTH 1

///////CALL STATES/////////
enum callStates {
    ACTIVE     = 0,
    HELD       = 1,
    DIALING    = 2,
    ALERTING   = 3,
    INCOMING   = 4,
    WAITING    = 5,
    DISCONNECT = 6,
};
///////////////////////////

// buttonsHelding() returns:
enum buttons {
    DECLINE = 0,
    SELECT  = 1,
    UP      = 2,
    DOWN    = 3,
    LEFT    = 4,
    RIGHT   = 5,
    ANSWER  = 6,
    LEFTFN  = ANSWER,
    RIGHTFN = DECLINE,
    BACK    = DECLINE,
    HOMESCR = DECLINE
};

// Text input charsets
enum charsets {
    SMALL_LATIN = 0,
    CAPS_LATIN  = 1,
    NUMBERS     = 2,
    HIRAGANA    = 3,
    KATAKANA    = 4,
    SMALL_UA    = 5,
    CAPS_UA     = 6,
    SMALL_RU    = 7,
    CAPS_RU     = 8,
    AUTO_CAPS
};

// ENABLE DEVELOPER MODE//
#define DEVMODE

// #define SIMDEBUG//

// CPU frequency for ESP32 by default is 240 MHz
#define FAST_CPU_FREQ_MHZ 240

// Lowest frequency for ESP32 is 20 MHZ
#define SLOW_CPU_FREQ_MHZ 20

// FONTS
#define FONT0 1

#define PROPERTIES_KEY_RESPATH "sys.res_path"
#define PROPERTIES_KEY_WALLPAPER "sys.wallpaper"
#define PROPERTIES_KEY_WALLPAPER_INDEX "sys.wallpaper_index"
#define PROPERTIES_KEY_WALLPAPER_ID "sys.wallpaper_id"
#define PROPERTIES_KEY_WALLPAPER_MODE "sys.wallpaper_mode"
#define PROPERTIES_KEY_WALLPAPER_PATH "sys.wallpaper_path"
#define PROPERTIES_KEY_DIB_MS "sys.dib_ms"
#define PROPERTIES_KEY_LANGUAGE "sys.language"

