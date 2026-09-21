#pragma once

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

void boot_i2c_init(void);

esp_err_t boot_i2c_write_reg(
    uint8_t address,
    uint8_t reg,
    uint8_t value
);

esp_err_t boot_i2c_read_reg(
    uint8_t address,
    uint8_t reg,
    uint8_t *data,
    size_t len
);

#ifdef __cplusplus
}
#endif