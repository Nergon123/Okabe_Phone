#include "okabe_fs.h"
#include <Platform/FileSystem/VFS.h>
#include <Platform/NString.h>

struct _CNFILE {
    NFile *nfile;
};

extern "C" {

CNFILE* ofs_open(const char *filename, const char *mode)
{
    NFile *nf;
    std::string nstr(filename);

    nf = VFS.open(nstr, mode);
    if (!nf)
        return NULL;
        
    struct _CNFILE *cnf = (struct _CNFILE*)malloc(sizeof(_CNFILE));
    if (!cnf)
        return NULL;
        
    cnf->nfile = nf;

    return cnf;
}

int ofs_close(CNFILE *file)
{
    file->nfile->close();
    delete file->nfile;
    return 0;
}
    
long ofs_read(void *buffer, size_t size, CNFILE *file)
{
    size_t readed;
    readed = file->nfile->read(buffer, size);
    return readed;
}

long ofs_write(void *buffer, size_t size, CNFILE *file)
{
    size_t bytes = file->nfile->write(buffer, size);
    return bytes;
}

int ofs_puts(const char* s, CNFILE *file)
{
    return (int)ofs_write((void*)s, strlen(s), file) - 1;
}

int ofs_remove(const char *filename)
{
    NString nstr =  NString(filename);
    // return -1 in case of error or 0
    return -(!VFS.remove(nstr));
}

long ofs_tell(CNFILE *file)
{
    long pos = file->nfile->position();
    return pos;
}

long ofs_size(CNFILE *file)
{
    return file->nfile->size();
}

long ofs_lseek(CNFILE *file, long pos, int whence)
{    
    int ret = file->nfile->seek(pos, whence);
    if (ret) {
        ret = file->nfile->position();
    } else {
        ret = -1;
    }
    
    return ret;
}

int ofs_gets(void *buffer, size_t size, CNFILE *file)
{
    int readed = file->nfile->readLine((char*)buffer, size);
    return -(!readed);
}


} /* extern "C" */
