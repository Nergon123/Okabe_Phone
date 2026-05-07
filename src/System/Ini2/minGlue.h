#include <CAPI/okabe_fs.h>
#include <stdio.h>
#include <string.h>

#define INI_BUFFERSIZE  256       /* maximum line length, maximum path length */
#define INI_FILETYPE    CNFILE*

static inline int ini_openread(const char *filename, INI_FILETYPE *file)
{
    if((*file = ofs_open((filename), "r")) == NULL)
        return 0;

    return 1;
}

static inline int ini_openwrite(const char *filename, INI_FILETYPE *file)
{
    if((*file = ofs_open((filename), "w")) == NULL)
        return 0;

    return 1;
}

#define ini_close(file)               (ofs_close(*(file)) == 0)
// #define ini_read(buffer,size,file)    (ofs_read((buffer), (size), *(file)) != 0)
// #define ini_read(buffer,size,file)    (ofs_gets((buffer), (size), *(file)) != 0)
#define ini_read(buffer,size,file)    (ini_read_block((buffer), (size), (file)))

static int ini_read_block(char *buffer, int size, INI_FILETYPE *file)
{
  size_t numread = size;
  char *eol;

  if ((numread = ofs_read(buffer, size, *file)) == 0)
    return 0;                   /* at EOF */
  if ((eol = strchr(buffer, '\n')) == NULL)
    eol = strchr(buffer, '\r');
  if (eol != NULL) {
    /* terminate the buffer */
    *++eol = '\0';
    /* "unread" the data that was read too much */
    ofs_lseek(*file, - (int)(numread - (size_t)(eol - buffer)), SEEK_CUR);
  } /* if */
  return 1;
}


// #define ini_write(buffer,file)        (ofs_puts((buffer), *(file)) != 0)
#define ini_write(buffer,file)        (ofs_write((buffer), strlen(buffer), *(file)) != 0)
#define ini_remove(filename)          (ofs_remove((filename)) == 0)

#define INI_FILEPOS                   size_t
#define ini_tell(file,pos)            (*(pos) = ofs_tell(*(file)))
#define ini_seek(file,pos)            (ofs_lseek(*(file), *(pos), SEEK_SET) != -1)

#ifndef INI_READONLY

static int ini_rename(char *source, char *dest)
{
  CNFILE *fr;
  CNFILE *fw;
  size_t n;

  if ((fr = ofs_open(source, "r")) == NULL)
    return 0;
  // if (ofs_remove(dest) != 0)
    // return 0;
  ofs_remove(dest);
  if ((fw = ofs_open(dest, "w")) == NULL)
    return 0;

  char buf[INI_BUFFERSIZE];
  while ((n=ofs_read(buf, INI_BUFFERSIZE, fr))) {
    ofs_write(buf, n, fw);
  }

  ofs_close(fr);
  ofs_close(fw);
  
  return ofs_remove(source) == 0;
}
#endif
