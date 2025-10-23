#pragma once
#include "../GlobalVariables.h"
#include "../Input/Input.h"

#include "../System/TextManipulation.h"
#include "../System/FontManagement.h"
#include "../System/Time.h"
#include "../UI/UIElements.h"
void ErrorWindow(NString reason);
bool confirmation(NString reason, NString yes = "Yes", NString no = "No");
void drawStatusBar(bool force = false);
