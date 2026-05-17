#ifndef _H_PROPERTIES
#define _H_PROPERTIES

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define PROPERTY_KEY_MAX 128
#define PROPERTY_VALUE_MAX 128

/* property_get: returns the length of the value which will never be
** greater than PROPERTY_VALUE_MAX - 1 and will always be zero terminated.
** (the length does not include the terminating zero).
**
** If the property read fails or returns an empty value, the default
** value is used (if nonnull).
*/
int property_get(const char *key, char *value, const char *default_value);

/* property_get_bool: returns the value of key coerced into a
** boolean. If the property is not set, then the default value is returned.
**
** If no property with this key is set (or the key is NULL) or the boolean
** conversion fails, the default value is returned.
**/
bool property_get_bool(const char *key, bool default_value);

/* property_get_long: returns the value of key truncated and coerced into a
** long. If the property is not set, then the default value is used.
**/
long property_get_long(const char *key, long default_value);

/* property_set: returns 0 on success, < 0 on failure
*/
int property_set(const char *key, const char *value);
int property_set_bool(const char *key, bool value);
int property_set_long(const char *key, long value);

#ifdef __cplusplus
}
#endif
#endif

