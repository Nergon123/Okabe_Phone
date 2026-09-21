/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <sys/reent.h>

#include "esp_log.h"
#include "sdkconfig.h"

#include "bootloader_common.h"
#include "bootloader_init.h"
#include "bootloader_utility.h"

#include "boot_i2c.h"
#include "boot_mcp23017.h"
#include "crash_reset.h"
#include "esp_rom_sys.h"
#include "soc/rtc_cntl_reg.h"

static const char *TAG = "boot";

static int select_partition_number(bootloader_state_t *bs);

/*
 * We arrive here after the ROM bootloader finished loading this
 * second-stage bootloader from flash.
 */
void __attribute__((noreturn)) call_start_cpu0(void) {
    /*
     * 1. Initialize hardware required by the bootloader.
     */
    if (bootloader_init() != ESP_OK) { bootloader_reset(); }

#ifdef CONFIG_BOOTLOADER_SKIP_VALIDATE_IN_DEEP_SLEEP

    /*
     * Note:
     * With this enabled, a deep-sleep wake may boot the previous
     * application before reaching our keypad check.
     */
    bootloader_utility_load_boot_image_from_deep_sleep();

#endif

    /*
     * 2. Select boot partition.
     */
    bootloader_state_t bs = {0};

    int boot_index = select_partition_number(&bs);

    if (boot_index == INVALID_INDEX) { bootloader_reset(); }

    ESP_LOGI(TAG, "Selected boot index: %d", boot_index);

    /*
     * 3. Load selected application.
     */
    bootloader_utility_load_boot_image(&bs, boot_index);

    /*
     * Never returns.
     */
}

/*
 * Select boot partition.
 */
static int select_partition_number(bootloader_state_t *bs) {
    /*
     * First load the partition table.
     *
     * This fills things like:
     *
     * bs->factory
     * bs->ota[]
     * bs->test
     * bs->ota_info
     */
    if (!bootloader_utility_load_partition_table(bs)) {
        ESP_LOGE(TAG, "Failed to load partition table");
        return INVALID_INDEX;
    }

    // Inspect the cause before any application startup clears the RTC hint.
    // Normal software restarts, cold boots and deep-sleep wakes stay normal.
    if (bs->factory.offset != 0 &&
        recovery_after_reset(esp_rom_get_reset_reason(0), REG_READ(RTC_CNTL_STORE6_REG))) {
        ESP_LOGW(TAG, "Abnormal reset: entering recovery diagnostics");
        return FACTORY_INDEX;
    }

    /*
     * Check keypad for recovery combo.
     */
    boot_i2c_init();

    bool recovery = false;

    esp_err_t err = boot_mcp23017_recovery_pressed(&recovery);

    if (err != ESP_OK) {
        /*
         * I2C/MCP failure should NOT brick normal boot.
         *
         * Just log it and continue to the normal application.
         */
        ESP_LOGW(TAG, "Could not read recovery keypad: %s", esp_err_to_name(err));
    }
    else if (recovery) {

        ESP_LOGW(TAG, "Recovery key combination detected");

        /*
         * Recovery is stored as:
         *
         *     app,factory
         *
         * bootloader_utility_load_partition_table()
         * puts that partition into bs->factory.
         */
        if (bs->factory.offset != 0) {
            ESP_LOGI(TAG, "Booting recovery at 0x%08lx", (unsigned long)bs->factory.offset);

            return FACTORY_INDEX;
        }

        ESP_LOGE(TAG, "Recovery requested, but factory partition does not exist");
    }

    /*
     * No recovery combo:
     * use ordinary OTA selection.
     */
    int selected = bootloader_utility_get_selected_boot_partition(bs);
    // Empty OTA metadata normally selects factory, which is our recovery app.
    // Without the recovery key combo, start the normal app in OTA slot zero.
    if (selected == FACTORY_INDEX && bs->ota[0].offset != 0) {
        return 0;
    }
    return selected;
}

#if CONFIG_LIBC_NEWLIB

struct _reent *__getreent(void) { return _GLOBAL_REENT; }

#endif