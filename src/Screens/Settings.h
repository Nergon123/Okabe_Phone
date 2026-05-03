#pragma once
#include "GlobalVariables.h"

#include "Input/Input.h"
#include "System/Files.h"
#include "System/FontManagement.h"
#include "System/Time.h"
#include "UI/ListMenu.h"
#include "UI/Notifications.h"
#include "UI/UIElements.h"
#include <Screens/ImageViewer.h>
void debugMenu();
void settings();
void setTime();
int  gallery();
void ringtoneSelector(bool isMail);
struct wallpaper {
    int       id;
    NString   path;
    ImageMode mode;
};
extern wallpaper currentWallpaper;
void             drawWallpaper();