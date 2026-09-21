#pragma once

#include <stdbool.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t boot_mcp23017_recovery_pressed(bool *pressed);

#ifdef __cplusplus
}
#endif