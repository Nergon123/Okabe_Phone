#include "properties.h"
#include <System/Ini2/minIni.h>

#define PROPERTY_INI_FILE_PATH "/spiffs/system/sysprop.ini"

int property_get(const char *key, char *value, const char *default_value)
{
    int len;

    len = ini_gets(NULL, key, default_value, value, PROPERTY_VALUE_MAX, PROPERTY_INI_FILE_PATH);

    return len;
}

bool property_get_bool(const char *key, bool default_value)
{
    bool val;

    val = ini_getbool(NULL, key, default_value, PROPERTY_INI_FILE_PATH);

    return val;
}

long property_get_long(const char *key, long default_value)
{
    long val;

    val = ini_getl(NULL, key, default_value, PROPERTY_INI_FILE_PATH);

    return val;
}

int property_set(const char *key, const char *value)
{
    int ret = -1;
    
    if (!key) {
        return ret;
    }

    ret = ini_puts(NULL, key, value, PROPERTY_INI_FILE_PATH);

    // convert (ok=1, err=0) to (ok=0, err=-1)
    ret = -(!ret);

    return ret;
}

int property_set_bool(const char *key, bool value)
{
    char *b;
    if (value == true) {
        b = "true";
    } else {
        b = "false";
    }

    return property_set(key, b);
}

int property_set_long(const char *key, long value)
{
    int ret;
    char buf[PROPERTY_VALUE_MAX];
    
    ret = snprintf(buf,PROPERTY_VALUE_MAX, "%ld", value);
    if (ret < 0) {
        return ret;
    }

    return property_set(key, buf);
}
