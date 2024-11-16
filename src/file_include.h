
#include <defines.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/semphr.h"
#include <WiFi.h>
#include <HTTPClient.h>
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

