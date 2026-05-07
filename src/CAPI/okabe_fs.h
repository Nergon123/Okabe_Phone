#ifndef _H_OKABE_FS
#define _H_OKABE_FS


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef struct _CNFILE CNFILE;

CNFILE* ofs_open(const char *filename, const char *mode);
int ofs_close(CNFILE *file);
long ofs_read(void *buffer, size_t size, CNFILE *file);
long ofs_write(void *buffer, size_t size, CNFILE *file);
int ofs_puts(const char* s, CNFILE *file);
long ofs_tell(CNFILE *file);
long ofs_size(CNFILE *file);
long ofs_lseek(CNFILE *file, long pos, int whence);
int ofs_gets(void *buffer, size_t size, CNFILE *file);

int ofs_remove(const char *filename);

#ifdef __cplusplus
}
#endif

#endif

