#pragma once
#include "Connectivity/SIM.h"
#include "GlobalVariables.h"
#ifdef INC_FREERTOS_H
extern TaskHandle_t TaskHCommand;
#elif defined(PC)
#include <thread>
#endif
void initBackgroundTasks();
void suspendCore(bool suspend);
void DeleteTask(TASK &task);
TASK LaunchTask(void (*function)(void *parameters), const char *name = "New Task",
                void *parameters = nullptr, int stackSize = 1024, int priority = 1, int core = 1);