#pragma once
#include <Arduino.h>
#include "../GlobalVariables.h"
#include "../Input/Input.h"
#include "../System/DrawGraphics.h"
#include "../System/TextManipulation.h"
#include "../System/FontManagement.h"
#include "../System/Time.h"
#include "../UI/UIElements.h"
void ErrorWindow(String reason);
bool confirmation(String reason);
void drawStatusBar(bool force = false);
