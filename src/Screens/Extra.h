#pragma once
#include "../UI/ListMenu.h"
#include "../System/Files.h"
#include "../System/FontManagement.h"
#include "../Input/Input.h"
#include "../Connectivity/_WiFi.h"
#include "../GlobalVariables.h"
#include "../System/Tasks.h"
#ifndef PC
#include <Arduino.h>
#include "esp_ota_ops.h"
#endif
void e();
void execute_application();