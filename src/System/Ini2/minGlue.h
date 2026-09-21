#ifndef OKABE_MININI_GLUE_H
#define OKABE_MININI_GLUE_H

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#define INI_BUFFERSIZE 256
#define INI_FILETYPE FILE *
#define INI_FILEPOS long

static inline int ini_openread(const char *filename, INI_FILETYPE *file) {
    *file = fopen(filename, "r");
    return *file != NULL;
}

static inline int ini_openwrite(const char *filename, INI_FILETYPE *file) {
    *file = fopen(filename, "w");
    return *file != NULL;
}

static inline int ini_close(INI_FILETYPE *file) {
    /* Writes throughout minIni are buffered; retain errors until commit. */
    int ok = !ferror(*file);
    if (fclose(*file) != 0) ok = 0;
    *file = NULL;
    return ok;
}

#define ini_read(buffer, size, file) (fgets((buffer), (size), *(file)) != NULL)
#define ini_write(buffer, file) (fputs((buffer), *(file)) >= 0)
#define ini_remove(filename) (remove(filename) == 0)
static inline int ini_tell(INI_FILETYPE *file, INI_FILEPOS *pos) {
    *pos = ftell(*file);
    return *pos >= 0;
}
#define ini_seek(file, pos) (fseek(*(file), *(pos), SEEK_SET) == 0)

#ifndef INI_READONLY
static inline int ini_rename(const char *source, const char *dest) {
    struct stat info;
    char backup[INI_BUFFERSIZE];
    size_t length = strlen(dest);
    if (rename(source, dest) == 0) return 1;

    /* SPIFFS rejects replacement of an existing name (IDF maps that error
     * to EIO). Move the old file aside rather than deleting its contents. */
    if (stat(dest, &info) != 0 || !S_ISREG(info.st_mode) ||
        length == 0 || length >= sizeof(backup)) return 0;
    memcpy(backup, dest, length + 1);
    backup[length - 1] = '^'; /* no extra length on SPIFFS's short names */
    if (strcmp(backup, dest) == 0 || strcmp(backup, source) == 0) return 0;
    if (stat(backup, &info) == 0 || errno != ENOENT) return 0;
    if (rename(dest, backup) != 0) return 0;
    if (rename(source, dest) != 0) {
        int saved_errno = errno;
        (void)rename(backup, dest); /* retain backup if restoring also fails */
        errno = saved_errno;
        return 0;
    }
    return remove(backup) == 0;
}
#endif
#endif
