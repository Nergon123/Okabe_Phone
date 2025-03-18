#ifndef DEFINES_H
#define DEFINES_H
#define DISABLE_ALL_LIBRARY_WARNINGS
#define RS485_RX_PIN 35
#define RS485_TX_PIN 26
#define RS485_RTS_PIN -1
#define SIM_INT_PIN 36 //33 (future)
#define chipSelect 13
#define MCP23017_ADDR 0x20
#define MAX_CONTACTS 100

// DELAY BETWEEN CHECKS

#define DIB_MS 500
//DELAY TO CONFIRM CHARACTER

#define INPUT_LOCATION_Y 240
//INPUT OF inputText() WINDOW

#define CWIDTH 1
//CURSOR WIDTH

#define FIRMVER "0.1.4 ALPHA"

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

#define BACK 0
#define SELECT 1
#define UP 2
#define DOWN 3
#define LEFT 4
#define RIGHT 5
#define ANSWER 6
#define DECLINE BACK
/////////////////////////
// ENABLE DEVELOPER MODE//
#define DEVMODE //
//#define SIMDEBUG//
/////////////////////////

//CPU frequency for ESP32 by default is 240 MHz  
#define FAST_CPU_FREQ_MHZ 240

//Lowest possible speed is 20 MHZ (?)
#define SLOW_CPU_FREQ_MHZ 20

#define RESOURCE_ADDRESS 0x5A708D

#define MAX_SD_FREQ 80000000
#define MIN_SD_FREQ 1000000 
#define SAFE_SD_FREQ 500000 
#endif
