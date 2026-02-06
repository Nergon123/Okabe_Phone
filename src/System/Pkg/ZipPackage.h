#ifndef _H_ZIPPACKAGE
#define _H_ZIPPACKAGE

#include "BasePackage.h"
#include <GlobalVariables.h>
#include <Platform/FileSystem/FileSystem.h>
#include <Platform/NString.h>
#include <System/Ini/INIReader.h>
#include <System/Zip/ZipFileProvider.h>

class ZipPackage : public BasePackage {
  public:
    enum Status {
        ZIPPKG_OK            = 0,
        ZIPPKG_BADZIPERR     = -1,
        ZIPPKG_BADCONTROLERR = -2,
        ZIPPKG_READERR       = -3,
    };

    ZipPackage();
    virtual ~ZipPackage();

    int setZip(IFile *file);
    int setZip(NString *path);

  protected:
    ZipFileProvider *m_pkgZipProvider = NULL;
    INIReader       *m_manifestParser = NULL;

  private:
    bool   m_fileOwnership = false;
    IFile *m_zipFile       = NULL;
    char   m_manifestBuf[PKG_MANIFEST_MAX_SIZE];

    void closeZipFile();
};

#endif
