#pragma once
#include "Connectivity/_WiFi.h"
#include "GlobalVariables.h"
#include "Input/Input.h"
#include "System/Files.h"
#include "System/FontManagement.h"
#include "System/Tasks.h"
#include "UI/ListMenu.h"
#include <Connectivity/OTA.h>
#ifndef PC
#include "esp_ota_ops.h"
#include <Arduino.h>
#endif
void e();
void execute_application();