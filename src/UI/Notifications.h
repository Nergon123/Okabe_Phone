#pragma once
#include "GlobalVariables.h"
#include "Input/Input.h"

#include "System/FontManagement.h"
#include "System/TextManipulation.h"
#include "System/Time.h"
#include "UI/UIElements.h"

enum NetworkStatusBarIcons {
    NSB_WIFI_0       = 0,
    NSB_WIFI_1       = 1,
    NSB_WIFI_2       = 2,
    NSB_WIFI_3       = 3,
    NSB_WIFI_4       = 4,
    NSB_SIM_3G       = 5,
    NSB_SIM_4G       = 6,
    NSB_SIM_EDGE     = 7,
    NSB_SIM_GPRS     = 8,
    NSB_SIM_IDLE     = 9,
    NSB_BT_IDLE      = 10,
    NSB_BT_CONNECTED = 11
};

void InfoWindow(NString reason,NString title = "ERROR",bool WaitForButton = true,uint16_t titlecolor = TFT_RED); 
bool confirmation(NString reason, NString yes = "Yes", NString no = "No");
void drawStatusBar(bool force = false);
