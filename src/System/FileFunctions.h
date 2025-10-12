#include "../UI/Notifications.h"
#include "Files.h"
#include "GlobalVariables.h"
typedef bool (*FileFunc)(NFile file);
struct FormatFunc {
    const char* actionName;
    const char* format;
    FileFunc    callback;
};

String getActionName(String fileName);
bool   NotSupported(NFile file); // other files
bool   ViewText(NFile file);     // txt, other text files
bool   ViewPNG(NFile file);      // png
bool   ApplyTheme(NFile file);   // nph
bool   FileOpen(NFile file);

FormatFunc funcFormats[] = {
    {.actionName = "View Text", .format = "txt;md", .callback = ViewText},
    {.actionName = "View Picture", .format = "png", .callback = ViewPNG},
    {.actionName = "Apply Theme", .format = "nph", .callback = ApplyTheme},

    {.actionName = "No action...", .format = "", .callback = NotSupported},
};