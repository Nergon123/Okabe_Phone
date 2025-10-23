#pragma once
#include "../GlobalVariables.h"
#include "../UI/ListMenu.h"
extern NString getActionName(NString fileName);
NString fileBrowser(NString path = "/", NString format = "*", bool graphical = true, NString title = "File Manager");