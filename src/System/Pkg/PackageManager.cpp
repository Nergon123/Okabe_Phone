#include "PackageManager.h"
#include <GlobalVariables.h>

static const char TAG[] = "PackageManager";

PackageManager::PackageManager()
{
    
}

PackageManager::~PackageManager()
{
    for (auto& kv : m_pkgMap) {
        kv.second->unload();
    }
}

const std::unordered_map<std::string, BasePackage *>&
    PackageManager::packages()
{
    return m_pkgMap;
}

int PackageManager::registerPackage(BasePackage *pkg)
{
    const bool ok = m_pkgMap.insert({pkg->getInfo()->id, pkg}).second;
    ESP_LOGI(TAG, "Register package \"%s\": %i", pkg->getInfo()->id.c_str(), ok);
    if (ok) {
        return 0;
    } else {
        return -1;
    }
}

int PackageManager::unregisterPackage(const std::string& id)
{
    auto search = m_pkgMap.find(id);
    if (search != m_pkgMap.end()) {
        search->second->unload();
        delete search->second;
        m_pkgMap.erase(search->first);
        return 0;
    } else {
        return -1;
    }
}

int PackageManager::runPackage(const std::string& id)
{
    int res = 0;
    auto kv = m_pkgMap.find(id);
    if (kv == m_pkgMap.end()) {
        ESP_LOGD(TAG, "Not found package \"%s\"!", id.c_str());
        return -1;  
    }

    res = kv->second->load();
    if (res < 0) {
        ESP_LOGD(TAG, "Load package \"%s\": %i", id.c_str(), res);
        return res;
    }

    res = kv->second->exec();
    if (res < 0) {
        ESP_LOGD(TAG, "Exec package \"%s\": %i", id.c_str(), res);
        return res;
    }

    kv->second->unload();
    return res;
}
