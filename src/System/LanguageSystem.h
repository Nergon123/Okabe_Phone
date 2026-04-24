
#include <Platform/NString.h>
#pragma once
enum class TextKey {
    UNKNOWN_STRING, // "[Text Error]"

    // RECOVERY
    RECOVERY_FAIL_NO_RES,      // "Seems that you flashed your device wrongly.Refer to the
                               // instructions for more information."
    RECOVERY_FAIL_FAIL_RES,    // "There was an error when loading resource file."
    RECOVERY_MANUAL,           // "Manually triggered recovery"
    RECOVERY_TITLE,            // "=RECOVERY=\n"
    RECOVERY_OPT_CHOOSE_RES,   // "Choose resource file"
    RECOVERY_TRY_AGAIN,        // "Try again"
    RECOVERY_TITLE_CHOOSE_ACT, // "Choose action."
    // RECOVERY END

    // BOOT
    BOOT_LOAD_RES,    // "Loading resource file..."
    BOOT_INIT_SDCARD, // "Initializing SDCard..."
    BOOT_INIT_SPIFFS, // "Initializing SPIFFS..."
    BOOT_CP_FILE_RAM, // "Copying file to RAM..."
    BOOT_INIT_RTOS,   // "Initializing RTOS tasks..."
    // BOOT END

    // RESOURCE SYSTEM
    RES_FAIL_FILE_HEADER,      // "Error when reading Main Header"
    RES_FAIL_MAGIC_MISMATCH,   // "Header Mismatch (Wrong file?)"
    RES_FAIL_VERSION_MISMATCH, // "File version mismatch."
    RES_FAIL_NO_IMAGES,        // "There is no Images..."
    RES_FAIL_IMGDATA,          // "Error reading ImageData"

    // "\nDRAWIMAGE:Imagebuffer Pointer is null\nprobably something wrong
    // with SPIRAM/PSRAM"
    RES_FAIL_NULL_IMGBUFFER,

    // "Resource file probably doesn't have\nchecksum!\n\nCRC anchor is
    // \"%s\"\nbut expected  \"%s\""
    RES_WARN_NO_CHECKSUM,

    RES_WARN_CHECKSUM_FAILURE, // "Checksum check failure:\n\nExpected   0x%08X\nCalculated 0x%08X"

    // RESOURCE SYSTEM END

    FILEMANAGER_SAVE_HERE,   // "Save Here"
    FILEMANAGER_PREV_FOLDER, // ".."

    // INFO WINDOW
    IW_TITLE_INFO,              // "Info"
    IW_TITLE_ERROR,             // "ERROR"
    IW_TASK_LIST_NA,            // "Task listing is not available."
    IW_FAIL_OPEN_IMG,           // "Failed to open Image!"
    IW_WIFI_CONNECTING,         // "Connecting..."
    IW_NOT_SUPPORTED,           // "Not supported..."
    IW_WIFI_EN_NEEDED,          // "You need to enable wifi first."
    IW_WIFI_SCANNING,           // "Scanning for WiFi Networks..."
    IW_WIFI_ISNT_EN,            // "Wifi is not enabled"
    IW_WIFI_ERR_IDLE,           // "Timeout (idle)"
    IW_WIFI_ERR_NO_SSID,        // "No SSID avaliable"
    IW_WIFI_ERR_SCAN_COMPLETED, // "Scan Completed"
    IW_WIFI_CONNECTED,          // "Connected!"
    IW_WIFI_ERR_CONN_FAIL,      // "Connection Failed"
    IW_WIFI_ERR_CONN_LOST,      // "Connection Lost"
    IW_WIFI_ERR_DISCONNECTED,   // "Disconnected."
    IW_WIFI_ERR_NO_SHIELD,      // "No WiFi shield"
    IW_WIFI_UNKNOWN,            // "Unknown Error (%d)."
    IW_INBOX_LOAD_MSGS,         // "Loading messages..."
    IW_APPLYING_THEME,          // "Applying theme..."
    IW_FILE_OPEN_FAILED,        // "Failed to open file..."
    IW_FILE_DOWNLOADED,         // "File downloaded!"
    // INFO WINDOW END

    LM_EMPTY, // "< EMPTY >"

    LM_WIFI,                   // "Wi-Fi"
    LM_WIFI_SETTINGS,          // "Wi-Fi Settings"
    WIFI_SSID,                 // "SSID"
    WIFI_PASSWORD,             // "Password"
    WIFI_CONNECT_TITLE,        // "Connect To Wi-Fi"
    WIFI_HOTSPOT_TOGGLE,       // "Wi-Fi Hotspot"
    WIFI_HOTSPOT_SETTINGS,     // "Hotspot settings"
    WIFI_SCAN_BUTTON,          // "Scan Wi-Fi networks"
    WIFI_TOGGLE_AUTOCONNECT,   // "Auto connect"
    WIFI_TOGGLE_AUTORECONNECT, // "Auto reconnect"
    WIFI_SET_HOSTNAME,         // "Set hostname"
    WIFI_HOSTNAME_FIELD,       // "Hostname"

    CANCEL_BUTTON,       // "Cancel"
    CONFIRM_BUTTON,      // "Confirm"
    SAVE_BUTTON,         // "Save"
    WIFI_CONNECT_BUTTON, // "Connect"

    LM_ADDRESS_BOOK,      // "Address Book"
    LM_TASKS_RUNNING,     // "Tasks"
    LM_TASKS_MANAGER,     // "Task Manager"
    LM_TASKS_EN_RAM_MON,  // "Enable RAM Monitor"
    LM_TASKS_DIS_RAM_MON, // "Disable RAM Monitor"

    MSGS_INBOX,            // "Inbox"
    MSGS_OUTBOX,           // "Outbox"
                           /////////////
    LM_SET_CHNG_WALLPAPER, // "Change Wallpaper"
    LM_SET_CALL_RINGTONE,  // "Set call ringtone"
    LM_SET_MAIL_RINGTONE,  // "Set mail ringtone"
    LM_SET_ADVANCED_SET,   // "Advanced Settings"

    LM_A_SET_SYSTEM,        // "System Settings"
    LM_A_SET_CONNECTIVITY,  //"Connectivity"
    LM_A_SET_LOOK_AND_FEEL, // "Look and feel"
    LM_A_SET_EXPERIMENTAL,  // "Experimental"

    LM_EXP_DEBUG_MENU, // "Debug Menu"

    LM_LAF_CHANGE_THEME, // "Change Theme"

    LM_SYS_DATE_TIME, // "Set Date & Time"
    MENU_DATE,        // "Date"
    MENU_TIME,        // "Time"

    LM_SYS_INPUT,   // "Input Settings"
    LM_INP_LAYOUTS, // "Layouts"
    LM_INP_TIMINGS, // "Timings"

    LM_SYS_LANG,  // "Language"
    LM_SYS_LANG_CHOOSE, // "Custom Language"
    // "Confirm Delay: %d ms"
    // NOTE: It meant to represent delay from choosing character
    // to actually printing it. "Confirm delay" can be confusing
    LM_INP_TIME_CONFIRM_DELAY,

    TEL_RECIEVING_CALL, // "Recieving call"
    TEL_CALLING,        // "Calling..."
    TEL_END_OF_CALL,    // "End of Call.."

    TEL_CONTACTS_OPT_CALL,     // "Call"
    TEL_CONTACTS_OPT_OUTGOING, // "Outgoing message"
    TEL_CONTACTS_OPT_EDIT,     // "Edit"
    TEL_CONTACTS_OPT_CREATE,   // "Create"
    TEL_CONTACTS_OPT_DELETE,   // "Delete"
    TEL_CONTACTS_NAME,         // "Name"
    TEL_CONTACTS_NUMBER,       // "Number"
    TEL_CONTACTS_EDIT,         // "Edit Entry"

    MSGS_OUTGOING,          // "Outgoing Mail"
    MSGS_OUT_MENU_CONTINUE, // "Continue"
    MSGS_OUT_MENU_SEND,     // "Send Message"
    MSGS_OUT_MENU_DELETE,   // "Delete"
    MSGS_OUT_MENU_SAVE,     // Save To Drafts

    MSGS_INCOMING,                // "Recieve Mail"
    MSGS_INC_MENU_REPLY,          // "Reply"
    MSGS_INC_MENU_DELETE,         // "Delete"
    MSGS_INC_MENU_DELETE_CONFIRM, // "Are you sure about deleting this message?"

    IMAGE_VIEWER, // "Image Viewer"

    LM_EXTRA,                // "Extra"
    LM_EXTRA_FILE_BROWSER,   // "File Browser"
    LM_EXTRA_VIEW_IMAGE,     // "View Image"
    LM_EXTRA_WALLPAPER_TEST, // "Wallpaper Modes"
    LM_EXTRA_SET_TIME,       // "Set Time"
    LM_EXTRA_TASK_MANAGER,   // "Task Manager"
    LM_EXTRA_PKG_MANAGER,    // "Package Manager"

    WALLPAPER_CENTERED,  // "Centered"
    WALLPAPER_TILED,     // "Tiled"
    WALLPAPER_FILLED,    // "Fill"
    WALLPAPER_STRETCHED, // "Streched"
    WALLPAPER_FIT_HOR,   // "Fit horizontally"
    WALLPAPER_FIT_VER,   // "Fit vertically"
    CONFIRM_YES,         // "Yes"
    CONFIRM_NO,          // "No"

    PKGS_CONF_REMOVE,        //"Remove package?"
    PKGS_CONF_REMOVED,       //"Package removed!"
    PKGS_ERR_FAIL_REMOVE,    //"Failed to remove the package""
    PKGS_ACT_OPT_RUN,        // "Run package"
    PKGS_ACT_OPT_INFO,       // "Info"
    PKGS_ACT_OPT_REMOVE,     // "Remove"
    PKGS_ACT_OPT_UNREGISTER, // "(DEBUG) Unregister"

    PKGS_INFOWINDOW, // "Name: %s\nID: %s\nVersion: %s"

    PKGS_LOAD_ERR, // "Package load error!"
    PKGS_LOADED,   // "Package loaded!"
    PKGS_ID_EXIST, // "ID already exist.\nPackage not loaded!"

    PKGMGR_MANAGE, // "Manage packages"
    PKGMGR_ADD,    // "Add new package"
    PKGMGR,        // "PkgMgr"

    LAST
};
void    resetLanguage();
NString getTranslation(TextKey id);
NString setLanguage(NString path); // returns error message