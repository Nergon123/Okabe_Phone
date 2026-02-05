#include "ZipFileProvider.h"

static const char TAG[] = "ZipFileProvider";

static void *okabeOpen(const char *filename, void *ud, int32_t *size) {
    // printf("Attempting to open %s\n", filename);
    IFile *f = (IFile *)ud;
    if (f) { *size = (int32_t)f->size(); }
    return (void *)f;
}

static void okabeClose(void *p) {
    ZIPFILE *pzf = (ZIPFILE *)p;
    if (!pzf) { return; }
    IFile *f = (IFile *)pzf->fHandle;
    f->close();
}

static int32_t okabeRead(void *p, uint8_t *buffer, int32_t length) {
    ZIPFILE *pzf = (ZIPFILE *)p;
    if (!pzf) { return 0; }
    IFile *f = (IFile *)pzf->fHandle;

    return (int32_t)f->read(buffer, length);
}

static int32_t okabeSeek(void *p, int32_t position, int type) {
    ZIPFILE *pzf = (ZIPFILE *)p;
    if (!pzf) { return 0; }
    IFile *f = (IFile *)pzf->fHandle;

    return f->seek(position, type);
}

ZipFileProvider::~ZipFileProvider() {
    if (m_isFileOpen) { this->close(); }

    if (m_isZipOpen) { closeZIP(); }
}

int ZipFileProvider::openZip(IFile *file) {
    int res;
    if (m_isZipOpen) { closeZIP(); }

    res = UNZIP::openZIP(file->name().c_str(), file, okabeOpen, okabeClose, okabeRead, okabeSeek);
    if (res == UNZ_OK) { m_isZipOpen = true; }

    return res;
}

int ZipFileProvider::setFile(NString *path) { return setFile(path->c_str()); }

int ZipFileProvider::setFile(const std::string &path) {
    int res = -1;

    if (!m_isZipOpen) { goto error; }

    if (m_isFileOpen) { closeCurrentFile(); }

    res = locateFile(path.c_str());
    ESP_LOGD(TAG, "locateFile: %i", res);
    if (res < 0) { goto error; }

    res = openCurrentFile();
    ESP_LOGD(TAG, "openCurrentFile: %i", res);
    if (res != UNZ_OK) { goto error; }

    res = UNZIP::getFileInfo(&m_fileInfo, NULL, NULL, NULL, NULL, NULL, 0);
    ESP_LOGD(TAG, "getFileInfo: %i", res);
    if (res < 0) { goto file_error; }

    m_isFileOpen = true;
    return UNZ_OK;

file_error:
    closeCurrentFile();

error:
    return res;
}

unz_file_info *ZipFileProvider::getFileInfo() {
    if (m_isFileOpen) { return &m_fileInfo; }
    else { return NULL; }
}

size_t ZipFileProvider::read(void *buf, size_t len) {
    if (m_isFileOpen) { return readCurrentFile((uint8_t *)buf, len); }
    else { return 0; }
}

void ZipFileProvider::close() {
    if (m_isFileOpen) {
        closeCurrentFile();
        m_isFileOpen = false;
    }
}

size_t ZipFileProvider::size() {
    if (m_isFileOpen) { return m_fileInfo.uncompressed_size; }
    else { return 0; }
}

bool ZipFileProvider::available() {
    if (m_isFileOpen) { return getCurrentFilePos() < m_fileInfo.uncompressed_size; }
    else { return false; }
}

size_t ZipFileProvider::position() {
    if (m_isFileOpen) { return getCurrentFilePos(); }
    else { return 0; }
}
