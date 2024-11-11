

#define RS485_RX_PIN 35
#define RS485_TX_PIN 26
#define RS485_RTS_PIN 4
#define chipSelect 13
#define MAX_CONTACTS 100

#define BACK 0
#define SELECT 1
#define UP 2
#define DOWN 3
/////////////////////////
//ENABLE DEVELOPER MODE//
#define DEVMODE      //
/////////////////////////



#include <ArduinoJson.h>
#include "BleMouse.h"
#include "BleKeyboard.h"
#include "SPI.h"
#include "TFT_eSPI.h"
#include "EXT/TFT_Config.h"
#include "EXT/IP5306.h"
#include "SD.h"
#include <Preferences.h>
#include <Wire.h>
#include "hal/uart_types.h"
#include "init.h"

#include "funct.h"
#include "render.h"
#include "screens.h"
#include <regex>

