#pragma once
#include "../GlobalVariables.h"
#include "../UI/ListMenu.h"
String fileBrowser(File dir = SD.open("/"),String format = "*",bool graphical = true);