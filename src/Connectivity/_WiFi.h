#pragma once
#include "../GlobalVariables.h"
#include "../UI/ListMenu.h"
#ifdef ARDUINO
#include <WiFi.h>
#endif
void WifiConnect(NString ssid, NString password);
void WiFiList();