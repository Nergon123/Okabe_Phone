
#include "ZipPackage.h"

ZipPackage::ZipPackage()
{
    m_pkgZipProvider = new ZipFileProvider();
}

ZipPackage::~ZipPackage()
{
    this->closeZipFile();

    if (m_pkgZipProvider) {
        delete m_pkgZipProvider;
        m_pkgZipProvider = NULL;
    }

    if (m_manifestParser) {
        delete m_manifestParser;
        m_manifestParser = NULL;
    }
}

int ZipPackage::setZip(IFile *file)
{
    int res = 0;
    size_t bytes;

    this->closeZipFile();
    m_zipFile = file;

    res = m_pkgZipProvider->openZip(file);
    if (res < 0)
        return ZIPPKG_BADZIPERR;

    res = m_pkgZipProvider->setFile("control.ini");
    if (res < 0)
        return ZIPPKG_BADCONTROLERR;

    memset(m_manifestBuf, 0, sizeof(m_manifestBuf));
    bytes = m_pkgZipProvider->read(m_manifestBuf, sizeof(m_manifestBuf));

    if (m_manifestParser) {
        delete m_manifestParser;
        m_manifestParser = NULL;
    }
        
    m_manifestParser =  new INIReader(m_manifestBuf, bytes);
    if (m_manifestParser->ParseError())
        return ZIPPKG_BADCONTROLERR;
        
    m_info.name = m_manifestParser->GetStringUnquoted("General","name","");
    if (m_info.name.empty())
        return ZIPPKG_BADCONTROLERR;
        
    m_info.id = m_manifestParser->GetStringUnquoted("General","id","");
    if (m_info.id.empty())
        return ZIPPKG_BADCONTROLERR;
            
    m_info.version = m_manifestParser->GetStringUnquoted("General","version","");
    if (m_info.version.empty())
        return ZIPPKG_BADCONTROLERR;

    return ZIPPKG_OK;
}

int ZipPackage::setZip(NString *path)
{
    IFile *f = VFS.open(path->c_str(), "r");
    if (!f)
        return ZIPPKG_BADZIPERR;
        
    m_fileOwnership = true;
    setZip(f);
    
    return ZIPPKG_OK;
}

void ZipPackage::closeZipFile()
{
    if (m_zipFile) {
        m_zipFile->close();
        if (m_fileOwnership) {
            delete m_zipFile;
            m_fileOwnership = false;
        }
        m_zipFile = NULL;
    }
}
