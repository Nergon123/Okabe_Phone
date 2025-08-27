#pragma once
#define FIRMVER "0.1.5 ALPHA"
#define REPOSITORY_LINK "https://github.com/Nergon123/Okabe_Phone"

#define LISTMENU_OPTIONS -2
#define LISTMENU_EXIT -1

#define SIM_RX_PIN 35
#define SIM_TX_PIN 26 
#define chipSelect 13
#define MCP23017_ADDR 0x20
#define MAX_CONTACTS 100

#define SimSerial Serial1

//For default serial communication
#define SERIAL_BAUD_RATE 115200
//For transfering data
#define FAST_SERIAL_BAUD_RATE 921600
//For SIM communication
#define SIM_BAUD_RATE 115200


#ifdef OKABE_PHONE_HW_0
#define SIM_INT_PIN 34
#define HAVE_AMPL
#else
#define SIM_INT_PIN 36 
#define VIBRO_PIN 34
#endif


#define AMPL_BCLK 12
#define AMPL_LRCLK 25
#define AMPL_DIN 19


//DELAY TO CONFIRM CHARACTER
#define DIB_MS 500

//INPUT OF inputText() WINDOW
#define INPUT_LOCATION_Y 240

//CURSOR WIDTH (PIXELS)
#define CWIDTH 1


//#define LOG

///////CALL STATES/////////

#define ACTIVE 0
#define HELD 1
#define DIALING 2
#define ALERTING 3
#define INCOMING 4 
#define WAITING 5
#define DISCONNECT 6
///////////////////////////

// buttonsHelding() returns:
#define BACK 0
#define SELECT 1
#define UP 2
#define DOWN 3
#define LEFT 4
#define RIGHT 5
#define ANSWER 6
#define DECLINE BACK

// ENABLE DEVELOPER MODE//
#define DEVMODE //

//#define SIMDEBUG//

//CPU frequency for ESP32 by default is 240 MHz  
#define FAST_CPU_FREQ_MHZ 240

//Lowest frequency for ESP32 is 20 MHZ
#define SLOW_CPU_FREQ_MHZ 20

//Resource offset 
#define RESOURCE_ADDRESS 0x5A708D


#define FAST_SD_FREQ 5000000
#define SAFE_SD_FREQ 5000000


//FONTS
#define FONT0 1
#define FONT1 FreeSans9pt7b
#define FONT2 FreeSansBold9pt7b
#define FONT3 FreeMono9pt7b
#define FONT4 FreeSans12pt7b

#define FILE_VERSION 1

#define RES_DEFAULT_WALLPAPER 0
#define RES_DEFAULT_