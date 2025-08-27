#pragma once

#include "GlobalVariables.h"
#include "UI/UIElements.h"
#include "Input/Input.h"
#include "Screens/Main.h"

#include "System/TextManipulation.h"
#include "UI/UIElements.h"
#include "System/Files.h"

bool initSDCard(bool fast);
void hardwareInit();
void storageInit();
void loadResource(ulong address, String resourcefile, uint8_t **_resources,int w=0,int h=0);
