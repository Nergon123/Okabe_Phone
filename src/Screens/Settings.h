#pragma once
#include "../GlobalVariables.h"

#include "../System/Files.h"
#include "../System/FontManagement.h"
#include "../UI/ListMenu.h"
#include "../UI/UIElements.h"
#include "../UI/Notifications.h"
#include "../Input/Input.h"
#include "../System/PNGDraw.h"
#include "../System/Time.h"
void settings();
void setTime(time_t *time);
int  gallery();
void ringtoneSelector(bool isMail);