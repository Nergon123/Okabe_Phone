
#include "PkgManagerApp.h"
#include <System/Pkg/LuaPackage.h>

static const char TAG[] = "PkgManagerApp";

static void removePopup(std::string& id)
{
    int res = -1;
    bool yes = confirmation(NString("Remove package?"));
    if (yes) {
        res = PS.removePackage(id);
        if (res == 0) {
            InfoWindow(NString("Package removed!"),
                    NString("Info"),
                    true,
                    TFT_GREEN);    
        } else {
            InfoWindow(NString("Failed to remove the package"));
        }
    }
}

static void actionPopup(int index)
{
    int choise = LISTMENU_NULL;
    const NString actionMenu[] = {
        "Run package",
        "Info",
        "Remove",
        "(DEBUG) Unregister"
    };

    BasePackage *pkg;
    BasePackageInfo *pkgInfo;

    choise = choiceMenu(actionMenu, ArraySize(actionMenu), true);
    if (choise < 0) {
        return;
    }

    auto it = pm.packages().begin();
    std::advance(it, index);

    pkg = it->second;
    pkgInfo = pkg->getInfo();
    
    switch (choise) {
    case 0: pm.runPackage(pkgInfo->id); break;
    case 1:
        InfoWindow(SplitString(NString("Name: ") + pkgInfo->name +
                    NString("\nID: ") + pkgInfo->id +
                    NString("\nVersion: ") + pkgInfo->version), NString("Info"), true, TFT_BLACK);
        break;
    case 2: removePopup(pkgInfo->id); break;
    case 3: pm.unregisterPackage(pkgInfo->id); break;
    }
}

static void manageMenu()
{
    int choice = LISTMENU_NULL;
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
        
        choice = listMenu(pkgList.data(), pkgList.size(), false, LM_SETTINGS, "Manage packages").index;
        if (choice >= 0) {
            actionPopup(choice);
        }
    }
}

static void loadMenu()
{
    int res = 0;
    NString path = fileBrowser("/", "|.lpkg|.zip|");
    if (path.isEmpty()) {
        return;
    }

    PS.installPackage(path.stdstr());
    return;

    auto p = new LuaPackage();
    res = p->setFile(&path);
    ESP_LOGD(TAG, "setFile: \"%s\", %i", path.c_str(), res);
    if (res < 0) {
        InfoWindow(NString("Package load error!"));
        return;
    }

    res = pm.registerPackage(p);
    if (res == 0) {
        InfoWindow(NString("Package loaded!"), NString("Info"), true, TFT_BLACK);
    } else {
        InfoWindow(NString(SplitString("ID already exist.\nPackage not loaded!")), NString("Info"), true, TFT_BLACK);
    }
}

int PkgMgr() {
    ESP_LOGD(TAG, "Start...");

    int choice = LISTMENU_NULL; 
    const NString mainMenu[] = {
        "Manage packages",
        "Add new package"
    };

    while (choice != LISTMENU_EXIT) {
        choice = listMenu(mainMenu, ArraySize(mainMenu), false, LM_SETTINGS, "PkgMgr").index;
        switch (choice) {
        case 0: manageMenu(); break;
        case 1: loadMenu(); break;
        }
    }
    
	currentRenderTarget->present();
	    
    return 0;
}

CREATE_SPKG(PkgMgr, "org.system.pkgmgr", PkgMgr);
