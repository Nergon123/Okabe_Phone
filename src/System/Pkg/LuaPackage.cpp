#include "LuaPackage.h"

static const char TAG[] = "LuaPackage";

LuaPackage::~LuaPackage()
{
    this->unload();
}

int LuaPackage::setFile(IFile *file)
{
    int res = 0;
    
    res = ZipPackage::setZip(file);
    if (res != PKG_OK) {
        return res;
    }

    res = m_pkgZipProvider->setFile("main.lua");
    if (res < 0) {
        return -1;
    }

    return PKG_OK;
}

int LuaPackage::setFile(NString *path)
{
    int res = 0;
    
    res = ZipPackage::setZip(path);
    ESP_LOGD(TAG, "setZip: %i", res);
    if (res != PKG_OK) {
        return res;
    }

    res = m_pkgZipProvider->setFile("main.lua");
    ESP_LOGD(TAG, "setFile: %i", res);
    if (res < 0) {
        return -10;
    }

    return PKG_OK;
}

int LuaPackage::load()
{
    if (m_vm) {
        delete m_vm;
        m_vm = NULL;
    }

    m_vm = new LuaVM();

    return PKG_OK;
}

int LuaPackage::exec()
{
    int res = PKG_ERR;

    res = m_pkgZipProvider->setFile("main.lua");
    if (res < 0) {
        return res;
    }
    
    if (m_vm) {
        res = m_vm->doFile(m_pkgZipProvider);
        return res;
    } else {
        return PKG_ERR;
    }
}

int LuaPackage::unload()
{
    if (m_vm) {
        delete m_vm;
        m_vm = NULL;
    }

    return PKG_OK;   
}
