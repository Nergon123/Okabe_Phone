#pragma once
#include "../GlobalVariables.h"
#include "../Connectivity/SIM.h"
#ifndef PC
extern TaskHandle_t TaskHCommand;
#endif
void initTasks();
void suspendCore(bool suspend) ;