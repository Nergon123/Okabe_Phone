#include "LanguageSystem.h"
#include <Defines.h>
#include <Platform/FileSystem/VFS.h>
#include <System/Ini/INIReader.h>
#include <unordered_map>
const char                          *LTAG              = "Lang";
std::unordered_map<TextKey, NString> text_placeholders = {
    {TextKey::UNKNOWN_STRING, "UNKNOWN_STRING"},
    {TextKey::RECOVERY_FAIL_NO_RES, "RECOVERY_FAIL_NO_RES"},
    {TextKey::RECOVERY_FAIL_FAIL_RES, "RECOVERY_FAIL_FAIL_RES"},
    {TextKey::RECOVERY_MANUAL, "RECOVERY_MANUAL"},
    {TextKey::RECOVERY_TITLE, "RECOVERY_TITLE"},
    {TextKey::RECOVERY_OPT_CHOOSE_RES, "RECOVERY_OPT_CHOOSE_RES"},
    {TextKey::RECOVERY_TRY_AGAIN, "RECOVERY_TRY_AGAIN"},
    {TextKey::RECOVERY_TITLE_CHOOSE_ACT, "RECOVERY_TITLE_CHOOSE_ACT"},
    {TextKey::BOOT_LOAD_RES, "BOOT_LOAD_RES"},
    {TextKey::BOOT_INIT_SDCARD, "BOOT_INIT_SDCARD"},
    {TextKey::BOOT_INIT_SPIFFS, "BOOT_INIT_SPIFFS"},
    {TextKey::BOOT_CP_FILE_RAM, "BOOT_CP_FILE_RAM"},
    {TextKey::BOOT_INIT_RTOS, "BOOT_INIT_RTOS"},
    {TextKey::RES_FAIL_FILE_HEADER, "RES_FAIL_FILE_HEADER"},
    {TextKey::RES_FAIL_MAGIC_MISMATCH, "RES_FAIL_MAGIC_MISMATCH"},
    {TextKey::RES_FAIL_VERSION_MISMATCH, "RES_FAIL_VERSION_MISMATCH"},
    {TextKey::RES_FAIL_NO_IMAGES, "RES_FAIL_NO_IMAGES"},
    {TextKey::RES_FAIL_IMGDATA, "RES_FAIL_IMGDATA"},
    {TextKey::RES_FAIL_NULL_IMGBUFFER, "RES_FAIL_NULL_IMGBUFFER"},
    {TextKey::RES_WARN_NO_CHECKSUM, "RES_WARN_NO_CHECKSUM"},
    {TextKey::RES_WARN_CHECKSUM_FAILURE, "RES_WARN_CHECKSUM_FAILURE"},
    {TextKey::FILEMANAGER_SAVE_HERE, "FILEMANAGER_SAVE_HERE"},
    {TextKey::FILEMANAGER_PREV_FOLDER, "FILEMANAGER_PREV_FOLDER"},
    {TextKey::IW_TITLE_INFO, "IW_TITLE_INFO"},
    {TextKey::IW_TITLE_ERROR, "IW_TITLE_ERROR"},
    {TextKey::IW_TASK_LIST_NA, "IW_TASK_LIST_NA"},
    {TextKey::IW_FAIL_OPEN_IMG, "IW_FAIL_OPEN_IMG"},
    {TextKey::IW_WIFI_CONNECTING, "IW_WIFI_CONNECTING"},
    {TextKey::IW_NOT_SUPPORTED, "IW_NOT_SUPPORTED"},
    {TextKey::IW_WIFI_EN_NEEDED, "IW_WIFI_EN_NEEDED"},
    {TextKey::IW_WIFI_SCANNING, "IW_WIFI_SCANNING"},
    {TextKey::IW_WIFI_ISNT_EN, "IW_WIFI_ISNT_EN"},
    {TextKey::IW_WIFI_ERR_IDLE, "IW_WIFI_ERR_IDLE"},
    {TextKey::IW_WIFI_ERR_NO_SSID, "IW_WIFI_ERR_NO_SSID"},
    {TextKey::IW_WIFI_ERR_SCAN_COMPLETED, "IW_WIFI_ERR_SCAN_COMPLETED"},
    {TextKey::IW_WIFI_CONNECTED, "IW_WIFI_CONNECTED"},
    {TextKey::IW_WIFI_ERR_CONN_FAIL, "IW_WIFI_ERR_CONN_FAIL"},
    {TextKey::IW_WIFI_ERR_CONN_LOST, "IW_WIFI_ERR_CONN_LOST"},
    {TextKey::IW_WIFI_ERR_DISCONNECTED, "IW_WIFI_ERR_DISCONNECTED"},
    {TextKey::IW_WIFI_ERR_NO_SHIELD, "IW_WIFI_ERR_NO_SHIELD"},
    {TextKey::IW_WIFI_UNKNOWN, "IW_WIFI_UNKNOWN"},
    {TextKey::IW_INBOX_LOAD_MSGS, "IW_INBOX_LOAD_MSGS"},
    {TextKey::IW_APPLYING_THEME, "IW_APPLYING_THEME"},
    {TextKey::IW_FILE_OPEN_FAILED, "IW_FILE_OPEN_FAILED"},
    {TextKey::IW_FILE_DOWNLOADED, "IW_FILE_DOWNLOADED"},
    {TextKey::LM_EMPTY, "LM_EMPTY"},
    {TextKey::LM_WIFI, "LM_WIFI"},
    {TextKey::LM_WIFI_SETTINGS, "LM_WIFI_SETTINGS"},
    {TextKey::WIFI_SSID, "WIFI_SSID"},
    {TextKey::WIFI_PASSWORD, "WIFI_PASSWORD"},
    {TextKey::WIFI_CONNECT_TITLE, "WIFI_CONNECT_TITLE"},
    {TextKey::WIFI_HOTSPOT_TOGGLE, "WIFI_HOTSPOT_TOGGLE"},
    {TextKey::WIFI_HOTSPOT_SETTINGS, "WIFI_HOTSPOT_SETTINGS"},
    {TextKey::WIFI_SCAN_BUTTON, "WIFI_SCAN_BUTTON"},
    {TextKey::WIFI_TOGGLE_AUTOCONNECT, "WIFI_TOGGLE_AUTOCONNECT"},
    {TextKey::WIFI_TOGGLE_AUTORECONNECT, "WIFI_TOGGLE_AUTORECONNECT"},
    {TextKey::WIFI_SET_HOSTNAME, "WIFI_SET_HOSTNAME"},
    {TextKey::WIFI_HOSTNAME_FIELD, "WIFI_HOSTNAME_FIELD"},
    {TextKey::CANCEL_BUTTON, "CANCEL_BUTTON"},
    {TextKey::CONFIRM_BUTTON, "CONFIRM_BUTTON"},
    {TextKey::SAVE_BUTTON, "SAVE_BUTTON"},
    {TextKey::WIFI_CONNECT_BUTTON, "WIFI_CONNECT_BUTTON"},
    {TextKey::LM_ADDRESS_BOOK, "LM_ADDRESS_BOOK"},
    {TextKey::LM_TASKS_RUNNING, "LM_TASKS_RUNNING"},
    {TextKey::LM_TASKS_MANAGER, "LM_TASKS_MANAGER"},
    {TextKey::LM_TASKS_EN_RAM_MON, "LM_TASKS_EN_RAM_MON"},
    {TextKey::LM_TASKS_DIS_RAM_MON, "LM_TASKS_DIS_RAM_MON"},
    {TextKey::MSGS_INBOX, "MSGS_INBOX"},
    {TextKey::MSGS_OUTBOX, "MSGS_OUTBOX"},
    {TextKey::LM_SET_CHNG_WALLPAPER, "LM_SET_CHNG_WALLPAPER"},
    {TextKey::LM_SET_CALL_RINGTONE, "LM_SET_CALL_RINGTONE"},
    {TextKey::LM_SET_MAIL_RINGTONE, "LM_SET_MAIL_RINGTONE"},
    {TextKey::LM_SET_ADVANCED_SET, "LM_SET_ADVANCED_SET"},
    {TextKey::LM_A_SET_SYSTEM, "LM_A_SET_SYSTEM"},
    {TextKey::LM_A_SET_CONNECTIVITY, "LM_A_SET_CONNECTIVITY"},
    {TextKey::LM_A_SET_LOOK_AND_FEEL, "LM_A_SET_LOOK_AND_FEEL"},
    {TextKey::LM_A_SET_EXPERIMENTAL, "LM_A_SET_EXPERIMENTAL"},
    {TextKey::LM_EXP_DEBUG_MENU, "LM_EXP_DEBUG_MENU"},
    {TextKey::LM_LAF_CHANGE_THEME, "LM_LAF_CHANGE_THEME"},
    {TextKey::LM_SYS_DATE_TIME, "LM_SYS_DATE_TIME"},
    {TextKey::MENU_DATE, "MENU_DATE"},
    {TextKey::MENU_TIME, "MENU_TIME"},
    {TextKey::LM_SYS_INPUT, "LM_SYS_INPUT"},
    {TextKey::LM_INP_LAYOUTS, "LM_INP_LAYOUTS"},
    {TextKey::LM_INP_TIMINGS, "LM_INP_TIMINGS"},
    {TextKey::LM_SYS_LANG, "LM_SYS_LANG"},
    {TextKey::LM_SYS_LANG_CHOOSE, "LM_SYS_LANG_CHOOSE"},
    {TextKey::LM_INP_TIME_CONFIRM_DELAY, "LM_INP_TIME_CONFIRM_DELAY"},
    {TextKey::TEL_RECIEVING_CALL, "TEL_RECIEVING_CALL"},
    {TextKey::TEL_CALLING, "TEL_CALLING"},
    {TextKey::TEL_END_OF_CALL, "TEL_END_OF_CALL"},
    {TextKey::TEL_CONTACTS_OPT_CALL, "TEL_CONTACTS_OPT_CALL"},
    {TextKey::TEL_CONTACTS_OPT_OUTGOING, "TEL_CONTACTS_OPT_OUTGOING"},
    {TextKey::TEL_CONTACTS_OPT_EDIT, "TEL_CONTACTS_OPT_EDIT"},
    {TextKey::TEL_CONTACTS_OPT_CREATE, "TEL_CONTACTS_OPT_CREATE"},
    {TextKey::TEL_CONTACTS_OPT_DELETE, "TEL_CONTACTS_OPT_DELETE"},
    {TextKey::TEL_CONTACTS_NAME, "TEL_CONTACTS_NAME"},
    {TextKey::TEL_CONTACTS_NUMBER, "TEL_CONTACTS_NUMBER"},
    {TextKey::TEL_CONTACTS_EDIT, "TEL_CONTACTS_EDIT"},
    {TextKey::MSGS_OUTGOING, "MSGS_OUTGOING"},
    {TextKey::MSGS_OUT_MENU_CONTINUE, "MSGS_OUT_MENU_CONTINUE"},
    {TextKey::MSGS_OUT_MENU_SEND, "MSGS_OUT_MENU_SEND"},
    {TextKey::MSGS_OUT_MENU_DELETE, "MSGS_OUT_MENU_DELETE"},
    {TextKey::MSGS_OUT_MENU_SAVE, "MSGS_OUT_MENU_SAVE"},
    {TextKey::MSGS_INCOMING, "MSGS_INCOMING"},
    {TextKey::MSGS_INC_MENU_REPLY, "MSGS_INC_MENU_REPLY"},
    {TextKey::MSGS_INC_MENU_DELETE, "MSGS_INC_MENU_DELETE"},
    {TextKey::MSGS_INC_MENU_DELETE_CONFIRM, "MSGS_INC_MENU_DELETE_CONFIRM"},
    {TextKey::IMAGE_VIEWER, "IMAGE_VIEWER"},
    {TextKey::LM_EXTRA, "LM_EXTRA"},
    {TextKey::LM_EXTRA_FILE_BROWSER, "LM_EXTRA_FILE_BROWSER"},
    {TextKey::LM_EXTRA_VIEW_IMAGE, "LM_EXTRA_VIEW_IMAGE"},
    {TextKey::LM_EXTRA_WALLPAPER_TEST, "LM_EXTRA_WALLPAPER_TEST"},
    {TextKey::LM_EXTRA_SET_TIME, "LM_EXTRA_SET_TIME"},
    {TextKey::LM_EXTRA_TASK_MANAGER, "LM_EXTRA_TASK_MANAGER"},
    {TextKey::LM_EXTRA_PKG_MANAGER, "LM_EXTRA_PKG_MANAGER"},
    {TextKey::WALLPAPER_CENTERED, "WALLPAPER_CENTERED"},
    {TextKey::WALLPAPER_TILED, "WALLPAPER_TILED"},
    {TextKey::WALLPAPER_FILLED, "WALLPAPER_FILLED"},
    {TextKey::WALLPAPER_STRETCHED, "WALLPAPER_STRETCHED"},
    {TextKey::WALLPAPER_FIT_HOR, "WALLPAPER_FIT_HOR"},
    {TextKey::WALLPAPER_FIT_VER, "WALLPAPER_FIT_VER"},
    {TextKey::CONFIRM_YES, "CONFIRM_YES"},
    {TextKey::CONFIRM_NO, "CONFIRM_NO"},
    {TextKey::PKGS_CONF_REMOVE, "PKGS_CONF_REMOVE"},
    {TextKey::PKGS_CONF_REMOVED, "PKGS_CONF_REMOVED"},
    {TextKey::PKGS_ERR_FAIL_REMOVE, "PKGS_ERR_FAIL_REMOVE"},
    {TextKey::PKGS_ACT_OPT_RUN, "PKGS_ACT_OPT_RUN"},
    {TextKey::PKGS_ACT_OPT_INFO, "PKGS_ACT_OPT_INFO"},
    {TextKey::PKGS_ACT_OPT_REMOVE, "PKGS_ACT_OPT_REMOVE"},
    {TextKey::PKGS_ACT_OPT_UNREGISTER, "PKGS_ACT_OPT_UNREGISTER"},
    {TextKey::PKGS_INFOWINDOW, "PKGS_INFOWINDOW"},
    {TextKey::PKGS_LOAD_ERR, "PKGS_LOAD_ERR"},
    {TextKey::PKGS_LOADED, "PKGS_LOADED"},
    {TextKey::PKGS_ID_EXIST, "PKGS_ID_EXIST"},
    {TextKey::PKGMGR_MANAGE, "PKGMGR_MANAGE"},
    {TextKey::PKGMGR_ADD, "PKGMGR_ADD"},
    {TextKey::PKGMGR, "PKGMGR"},

};

std::unordered_map<TextKey, NString> english = {
    {TextKey::UNKNOWN_STRING, "[Text Error]"},
    {TextKey::RECOVERY_FAIL_NO_RES,
     "Seems that you flashed your device wrongly.Refer to the instructions for more information."},
    {TextKey::RECOVERY_FAIL_FAIL_RES, "There was an error when loading resource file."},
    {TextKey::RECOVERY_MANUAL, "Manually triggered recovery"},
    {TextKey::RECOVERY_TITLE, "=RECOVERY=\n"},
    {TextKey::RECOVERY_OPT_CHOOSE_RES, "Choose resource file"},
    {TextKey::RECOVERY_TRY_AGAIN, "Try again"},
    {TextKey::RECOVERY_TITLE_CHOOSE_ACT, "Choose action."},
    {TextKey::BOOT_LOAD_RES, "Loading resource file..."},
    {TextKey::BOOT_INIT_SDCARD, "Initializing SDCard..."},
    {TextKey::BOOT_INIT_SPIFFS, "Initializing SPIFFS..."},
    {TextKey::BOOT_CP_FILE_RAM, "Copying file to RAM..."},
    {TextKey::BOOT_INIT_RTOS, "Initializing RTOS tasks..."},
    {TextKey::RES_FAIL_FILE_HEADER, "Error when reading Main Header"},
    {TextKey::RES_FAIL_MAGIC_MISMATCH, "Header Mismatch (Wrong file?)"},
    {TextKey::RES_FAIL_VERSION_MISMATCH, "File version mismatch."},
    {TextKey::RES_FAIL_NO_IMAGES, "There is no Images..."},
    {TextKey::RES_FAIL_IMGDATA, "Error reading ImageData"},
    {TextKey::RES_FAIL_NULL_IMGBUFFER,
     "\nDRAWIMAGE:Imagebuffer Pointer is null\nprobably something wrong with SPIRAM/PSRAM"},
    {TextKey::RES_WARN_NO_CHECKSUM, "Resource file probably doesn't have\nchecksum!\n\nCRC anchor "
                                    "is \"%s\"\nbut expected  \"%s\""},
    {TextKey::RES_WARN_CHECKSUM_FAILURE,
     "Checksum check failure:\n\nExpected   0x%08X\nCalculated 0x%08X"},
    {TextKey::FILEMANAGER_SAVE_HERE, "Save Here"},
    {TextKey::FILEMANAGER_PREV_FOLDER, ".."},
    {TextKey::IW_TITLE_INFO, "Info"},
    {TextKey::IW_TITLE_ERROR, "ERROR"},
    {TextKey::IW_TASK_LIST_NA, "Task listing is not available."},
    {TextKey::IW_FAIL_OPEN_IMG, "Failed to open Image!"},
    {TextKey::IW_WIFI_CONNECTING, "Connecting..."},
    {TextKey::IW_NOT_SUPPORTED, "Not supported..."},
    {TextKey::IW_WIFI_EN_NEEDED, "You need to enable wifi first."},
    {TextKey::IW_WIFI_SCANNING, "Scanning for WiFi Networks..."},
    {TextKey::IW_WIFI_ISNT_EN, "Wifi is not enabled"},
    {TextKey::IW_WIFI_ERR_IDLE, "Timeout (idle)"},
    {TextKey::IW_WIFI_ERR_NO_SSID, "No SSID avaliable"},
    {TextKey::IW_WIFI_ERR_SCAN_COMPLETED, "Scan Completed"},
    {TextKey::IW_WIFI_CONNECTED, "Connected!"},
    {TextKey::IW_WIFI_ERR_CONN_FAIL, "Connection Failed"},
    {TextKey::IW_WIFI_ERR_CONN_LOST, "Connection Lost"},
    {TextKey::IW_WIFI_ERR_DISCONNECTED, "Disconnected."},
    {TextKey::IW_WIFI_ERR_NO_SHIELD, "No WiFi shield"},
    {TextKey::IW_WIFI_UNKNOWN, "Unknown Error (%d)."},
    {TextKey::IW_INBOX_LOAD_MSGS, "Loading messages..."},
    {TextKey::IW_APPLYING_THEME, "Applying theme..."},
    {TextKey::IW_FILE_OPEN_FAILED, "Failed to open file..."},
    {TextKey::IW_FILE_DOWNLOADED, "File downloaded!"},
    {TextKey::LM_EMPTY, "< EMPTY >"},
    {TextKey::LM_WIFI, "Wi-Fi"},
    {TextKey::LM_WIFI_SETTINGS, "Wi-Fi Settings"},
    {TextKey::WIFI_SSID, "SSID"},
    {TextKey::WIFI_PASSWORD, "Password"},
    {TextKey::WIFI_CONNECT_TITLE, "Connect To Wi-Fi"},
    {TextKey::WIFI_HOTSPOT_TOGGLE, "Wi-Fi Hotspot"},
    {TextKey::WIFI_HOTSPOT_SETTINGS, "Hotspot settings"},
    {TextKey::WIFI_SCAN_BUTTON, "Scan Wi-Fi networks"},
    {TextKey::WIFI_TOGGLE_AUTOCONNECT, "Auto connect"},
    {TextKey::WIFI_TOGGLE_AUTORECONNECT, "Auto reconnect"},
    {TextKey::WIFI_SET_HOSTNAME, "Set hostname"},
    {TextKey::WIFI_HOSTNAME_FIELD, "Hostname"},
    {TextKey::CANCEL_BUTTON, "Cancel"},
    {TextKey::CONFIRM_BUTTON, "Confirm"},
    {TextKey::SAVE_BUTTON, "Save"},
    {TextKey::WIFI_CONNECT_BUTTON, "Connect"},
    {TextKey::LM_ADDRESS_BOOK, "Address Book"},
    {TextKey::LM_TASKS_RUNNING, "Tasks"},
    {TextKey::LM_TASKS_MANAGER, "Task Manager"},
    {TextKey::LM_TASKS_EN_RAM_MON, "Enable RAM Monitor"},
    {TextKey::LM_TASKS_DIS_RAM_MON, "Disable RAM Monitor"},
    {TextKey::MSGS_INBOX, "Inbox"},
    {TextKey::MSGS_OUTBOX, "Outbox"},
    {TextKey::LM_SET_CHNG_WALLPAPER, "Change Wallpaper"},
    {TextKey::LM_SET_CALL_RINGTONE, "Set call ringtone"},
    {TextKey::LM_SET_MAIL_RINGTONE, "Set mail ringtone"},
    {TextKey::LM_SET_ADVANCED_SET, "Advanced Settings"},
    {TextKey::LM_A_SET_SYSTEM, "System Settings"},
    {TextKey::LM_A_SET_CONNECTIVITY, "Connectivity"},
    {TextKey::LM_A_SET_LOOK_AND_FEEL, "Look and feel"},
    {TextKey::LM_A_SET_EXPERIMENTAL, "Experimental"},
    {TextKey::LM_EXP_DEBUG_MENU, "Debug Menu"},
    {TextKey::LM_LAF_CHANGE_THEME, "Change Theme"},
    {TextKey::LM_SYS_DATE_TIME, "Set Date & Time"},
    {TextKey::MENU_DATE, "Date"},
    {TextKey::MENU_TIME, "Time"},
    {TextKey::LM_SYS_INPUT, "Input Settings"},
    {TextKey::LM_INP_LAYOUTS, "Layouts"},
    {TextKey::LM_INP_TIMINGS, "Timings"},
    {TextKey::LM_SYS_LANG, "Language"},
    {TextKey::LM_SYS_LANG_CHOOSE, "Custom Language"},
    {TextKey::LM_INP_TIME_CONFIRM_DELAY, "Confirm Delay: %d ms"},
    {TextKey::TEL_RECIEVING_CALL, "Recieving call"},
    {TextKey::TEL_CALLING, "Calling..."},
    {TextKey::TEL_END_OF_CALL, "End of Call.."},
    {TextKey::TEL_CONTACTS_OPT_CALL, "Call"},
    {TextKey::TEL_CONTACTS_OPT_OUTGOING, "Outgoing message"},
    {TextKey::TEL_CONTACTS_OPT_EDIT, "Edit"},
    {TextKey::TEL_CONTACTS_OPT_CREATE, "Create"},
    {TextKey::TEL_CONTACTS_OPT_DELETE, "Delete"},
    {TextKey::TEL_CONTACTS_NAME, "Name"},
    {TextKey::TEL_CONTACTS_NUMBER, "Number"},
    {TextKey::TEL_CONTACTS_EDIT, "Edit Entry"},
    {TextKey::MSGS_OUTGOING, "Outgoing Mail"},
    {TextKey::MSGS_OUT_MENU_CONTINUE, "Continue"},
    {TextKey::MSGS_OUT_MENU_SEND, "Send Message"},
    {TextKey::MSGS_OUT_MENU_DELETE, "Delete"},
    {TextKey::MSGS_OUT_MENU_SAVE, "Save To Drafts"},
    {TextKey::MSGS_INCOMING, "Recieve Mail"},
    {TextKey::MSGS_INC_MENU_REPLY, "Reply"},
    {TextKey::MSGS_INC_MENU_DELETE, "Delete"},
    {TextKey::MSGS_INC_MENU_DELETE_CONFIRM, "Are you sure about deleting this message?"},
    {TextKey::IMAGE_VIEWER, "Image Viewer"},
    {TextKey::LM_EXTRA, "Extra"},
    {TextKey::LM_EXTRA_FILE_BROWSER, "File Browser"},
    {TextKey::LM_EXTRA_VIEW_IMAGE, "View Image"},
    {TextKey::LM_EXTRA_WALLPAPER_TEST, "Wallpaper Modes"},
    {TextKey::LM_EXTRA_SET_TIME, "Set Time"},
    {TextKey::LM_EXTRA_TASK_MANAGER, "Task Manager"},
    {TextKey::LM_EXTRA_PKG_MANAGER, "Package Manager"},
    {TextKey::WALLPAPER_CENTERED, "Centered"},
    {TextKey::WALLPAPER_TILED, "Tiled"},
    {TextKey::WALLPAPER_FILLED, "Fill"},
    {TextKey::WALLPAPER_STRETCHED, "Streched"},
    {TextKey::WALLPAPER_FIT_HOR, "Fit horizontally"},
    {TextKey::WALLPAPER_FIT_VER, "Fit vertically"},
    {TextKey::CONFIRM_YES, "Yes"},
    {TextKey::CONFIRM_NO, "No"},
    {TextKey::PKGS_CONF_REMOVE, "Remove package?"},
    {TextKey::PKGS_CONF_REMOVED, "Package removed!"},
    {TextKey::PKGS_ERR_FAIL_REMOVE, "Failed to remove the package"},
    {TextKey::PKGS_ACT_OPT_RUN, "Run package"},
    {TextKey::PKGS_ACT_OPT_INFO, "Info"},
    {TextKey::PKGS_ACT_OPT_REMOVE, "Remove"},
    {TextKey::PKGS_ACT_OPT_UNREGISTER, "(DEBUG) Unregister"},
    {TextKey::PKGS_INFOWINDOW, "Name: %s\nID: %s\nVersion: %s"},
    {TextKey::PKGS_LOAD_ERR, "Package load error!"},
    {TextKey::PKGS_LOADED, "Package loaded!"},
    {TextKey::PKGS_ID_EXIST, "ID already exist.\nPackage not loaded!"},
    {TextKey::PKGMGR_MANAGE, "Manage packages"},
    {TextKey::PKGMGR_ADD, "Add new package"},
    {TextKey::PKGMGR, "PkgMgr"},
};

std::unordered_map<TextKey, NString> custom;

NString getTranslation(TextKey id) {
    NString translation = ">>NO STRING<<";
    try {
        translation = text_placeholders.at(id);
    } catch (std::exception &e3) {
        ESP_LOGE(LTAG, "NO PLACEHOLDER(%s) | ID: %d", e3.what(), (int)id);
    }
    try {
        translation = custom.at(id);

    } catch (std::exception &e1) {
        if (custom.size() > 0) {
            ESP_LOGE(LTAG, "TRY 1:%s | ID: %d | %s", e1.what(), (int)id, translation.c_str());
        }
        try {
            translation = english.at(id);
        } catch (std::exception &e2) { ESP_LOGE(LTAG, "TRY 2:%s | ID: %d", e2.what(), (int)id); }
    }

    return translation;
}
NString workSTR(const NString &str) {
    NString output;
    int     length = str.length();

    if (length == 0) { return output; }
    output.reserve(length);
    int start = 0;
    int end   = length;

    if (length >= 2 && str[0] == '"' && str[length - 1] == '"') {
        start = 1;
        end   = length - 1;
    }

    for (int i = start; i < end; i++) {
        char ch = str[i];

        if (ch == '\\' && i + 1 < end) {
            char next = str[i + 1];

            switch (next) {
            case 'n': output += '\n'; break;
            case 't': output += '\t'; break;
            case '\\': output += '\\'; break;
            case '"': output += '"'; break;
            case 'r': output += '\r'; break;
            default: output += next; break;
            }

            i++;
        }
        else { output += ch; }
    }

    return output;
}

void    resetLanguage() { custom.clear(); }
NString setLanguage(NString path) {
    NFile *file     = VFS.open(path);
    size_t filesize = file->size();
    char  *text     = (char *)ps_malloc(filesize);
    size_t read     = file->read(text, filesize);
    file->close();
    if (filesize != read) {
        ESP_LOGE(LTAG, "ERROR when reading file!");
        return "ERROR when reading file!";
    }
    INIReader reader(text, filesize);
    if (reader.ParseError()) {
        ESP_LOGE(LTAG, "%s", reader.ParseErrorMessage().c_str());
        return reader.ParseErrorMessage();
    };

    for (int i = 0; i < (int)TextKey::LAST; i++) {
        NString text =
            reader.GetString("translation", text_placeholders.at((TextKey)i), "__NULLL__");
        text = workSTR(text);
        if (text == "__NULLL__") {
            ESP_LOGW(LTAG, "Missing translation entry for %s", text_placeholders.at((TextKey)i));
            continue;
        }

        try {
            custom.erase((TextKey)i);
        } catch (std::exception &ex) {};

        try {
            custom.insert({(TextKey)i, text});
        } catch (std::exception &ex) {
            ESP_LOGE(LTAG, "Failed to replace TextKey %d, %s", i, ex.what());
        }
    }
    free(text);
    return "";
}