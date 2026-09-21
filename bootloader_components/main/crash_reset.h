#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_system.h"
#include "soc/reset_reasons.h"

// ESP-IDF 5.5.x ESP32/S3 stores the panic hint twice in RTC STORE6, with
// bit 31 set. Read only: the application reset-reason constructor consumes it.
static inline bool recovery_after_reset(unsigned raw, uint32_t stored_hint) {
    unsigned low = stored_hint & 0x7fff;
    unsigned high = (stored_hint >> 16) & 0x7fff;
    if (raw == RESET_REASON_CORE_SW || raw == RESET_REASON_CPU0_SW) {
        return (stored_hint & 0x80000000u) && low == high &&
            (low == ESP_RST_PANIC || low == ESP_RST_INT_WDT ||
             low == ESP_RST_TASK_WDT || low == ESP_RST_BROWNOUT);
    }
    switch (raw) {
    case RESET_REASON_CORE_MWDT0:
    case RESET_REASON_CORE_MWDT1:
    case RESET_REASON_CORE_RTC_WDT:
    case RESET_REASON_CPU0_MWDT0:
    case RESET_REASON_CPU0_RTC_WDT:
    case RESET_REASON_SYS_RTC_WDT:
    case RESET_REASON_SYS_BROWN_OUT:
        return true;
#ifdef CONFIG_IDF_TARGET_ESP32S3
    case RESET_REASON_CPU0_MWDT1:
    case RESET_REASON_SYS_SUPER_WDT:
        return true;
#endif
    default:
        return false;
    }
}
