#pragma once
#include <Arduino.h>
#include "../GlobalVariables.h"
#include "../Connectivity/SIM.h"
#include "../Connectivity/Serial.h"
void initTasks();
void suspendCore(bool suspend) ;