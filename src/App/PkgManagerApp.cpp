
#include "PkgManagerApp.h"
#include <System/Pkg/LuaPackage.h>

static const char TAG[] = "PkgManagerApp";

static void removePopup(std::string &id) {
    int  res = -1;
    bool yes = confirmation(getTranslation(TextKey::PKGS_CONF_REMOVE));
    if (yes) {
        res = PaStor.removePackage(id);
        if (res == 0) {
            InfoWindow(getTranslation(TextKey::PKGS_CONF_REMOVED),getTranslation(TextKey::IW_TITLE_INFO), true, TFT_GREEN);
        }
        else { InfoWindow(getTranslation(TextKey::PKGS_ERR_FAIL_REMOVE)); }
    }
}

static void actionPopup(int index) {
    int           choise       = LISTMENU_NULL;
    const NString actionMenu[] = {getTranslation(TextKey::PKGS_ACT_OPT_RUN), getTranslation(TextKey::PKGS_ACT_OPT_INFO), getTranslation(TextKey::PKGS_ACT_OPT_REMOVE), getTranslation(TextKey::PKGS_ACT_OPT_UNREGISTER)};

    BasePackage     *pkg;
    BasePackageInfo *pkgInfo;

    choise = choiceMenu(actionMenu, ArraySize(actionMenu), true);
    if (choise < 0) { return; }

    auto it = pm.packages().begin();
    std::advance(it, index);

    pkg     = it->second;
    pkgInfo = pkg->getInfo();

    switch (choise) {
    case 0: pm.runPackage(pkgInfo->id); break;
    case 1:
        InfoWindow(SplitString(NString::format(getTranslation(TextKey::PKGS_INFOWINDOW).c_str(),
                                               pkgInfo->name.c_str(), pkgInfo->id.c_str(), pkgInfo->version.c_str())),
                   getTranslation(TextKey::IW_TITLE_INFO), true, TFT_BLACK);
        break;
    case 2: removePopup(pkgInfo->id); break;
    case 3: pm.unregisterPackage(pkgInfo->id); break;
    }
}

static void manageMenu() {
    int                  choice = LISTMENU_NULL;
    std::vector<NString> pkgList;

    while (choice != LISTMENU_EXIT) {
        // ESP_LOGD(TAG, "pm size: %i",pm.packages().size());
        pkgList.clear();
        pkgList.reserve(pm.packages().size());
        BasePackageInfo *info;
        for (auto kv : pm.packages()) {
            info = kv.second->getInfo();

            pkgList.push_back(NString(info->name) + " (" + NString(info->id) + ")");
        }
        // ESP_LOGD(TAG, "pkgList size: %i",pkgList.size());

        choice =
            listMenu(pkgList.data(), pkgList.size(), false, LM_SETTINGS, getTranslation(TextKey::PKGMGR_MANAGE)).index;
        if (choice >= 0) { actionPopup(choice); }
    }
}

static void loadMenu() {
    int     res  = 0;
    NString path = fileBrowser("/", "|.lpkg|.zip|");
    if (path.isEmpty()) { return; }

    PaStor.installPackage(path.stdstr());
    return;

    auto p = new LuaPackage();
    res    = p->setFile(&path);
    ESP_LOGD(TAG, "setFile: \"%s\", %i", path.c_str(), res);
    if (res < 0) {
        InfoWindow(getTranslation(TextKey::PKGS_LOAD_ERR));
        return;
    }

    res = pm.registerPackage(p);
    if (res == 0) { InfoWindow(getTranslation(TextKey::PKGS_LOADED), getTranslation(TextKey::IW_TITLE_INFO), true, TFT_BLACK); }
    else {
        InfoWindow(getTranslation(TextKey::PKGS_ID_EXIST), getTranslation(TextKey::IW_TITLE_INFO),
                   true, TFT_BLACK);
    }
}

int PkgMgr() {
    ESP_LOGD(TAG, "Start...");

    int           choice     = LISTMENU_NULL;
    const NString mainMenu[] = {getTranslation(TextKey::PKGMGR_MANAGE), getTranslation(TextKey::PKGMGR_ADD)};

    while (choice != LISTMENU_EXIT) {
        choice = listMenu(mainMenu, ArraySize(mainMenu), false, LM_SETTINGS, getTranslation(TextKey::PKGMGR)).index;
        switch (choice) {
        case 0: manageMenu(); break;
        case 1: loadMenu(); break;
        }
    }

    currentRenderTarget->present();

    return 0;
}

CREATE_SPKG(PkgMgr, "org.system.pkgmgr", PkgMgr);
