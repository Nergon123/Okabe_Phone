#pragma once

#define FIRMVER         "0.1.6 ALPHA"
#define REPOSITORY_LINK "https://github.com/Nergon123/Okabe_Phone"

#define LISTMENU_NULL    -3
#define LISTMENU_OPTIONS -2
#define LISTMENU_EXIT    -1
#define LISTMENU_BUFFER  true

#define SIM_RX_PIN    35
#define SIM_TX_PIN    26

#define MAX_CONTACTS  100



#ifdef PC

#define PROGMEM 

#ifndef ESP_LOGG
#define ESP_LOGG(level, tag, format, ...) ::printf("[%s][%s] " format "\n", level, tag, ##__VA_ARGS__)
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

#endif

// delay between SIM card checks
#define DBC_MS 5000

#define SimSerial Serial1



#define lines_to_draw_wo_psram 3



#ifdef OKABE_PHONE_HW_0
#define SIM_INT_PIN 34
#define HAVE_AMPL
#else
#define SIM_INT_PIN 36
#define VIBRO_PIN   34
#endif

#define AMPL_BCLK  12
#define AMPL_LRCLK 25
#define AMPL_DIN   19

// DELAY TO CONFIRM CHARACTER
#define DIB_MS 500

// INPUT OF inputText() WINDOW
#define INPUT_LOCATION_Y 240

// CURSOR WIDTH (PIXELS)
#define CWIDTH 1

// #define LOG

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
    BACK    = 0,
    SELECT  = 1,
    UP      = 2,
    DOWN    = 3,
    LEFT    = 4,
    RIGHT   = 5,
    ANSWER  = 6,
    DECLINE = BACK,
};
// ENABLE DEVELOPER MODE//
#define DEVMODE //

// #define SIMDEBUG//

// CPU frequency for ESP32 by default is 240 MHz
#define FAST_CPU_FREQ_MHZ 240

// Lowest frequency for ESP32 is 20 MHZ
#define SLOW_CPU_FREQ_MHZ 20

#define FAST_SD_FREQ 20 * 1000 * 1000
#define SAFE_SD_FREQ 1 * 1000 * 1000

// FONTS
#define FONT0 1
#define FONT1 FreeSans9pt7b
#define FONT2 FreeSansBold9pt7b
#define FONT3 FreeMono9pt7b
#define FONT4 FreeSans12pt7b

#define FILE_VERSION 1
