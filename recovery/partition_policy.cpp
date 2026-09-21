#include "partition_policy.h"
#include <cstring>
namespace recovery {
bool canUpdate(const esp_partition_t* p, const esp_partition_t* running) {
    return p && running && p->type == ESP_PARTITION_TYPE_APP && !p->readonly &&
        p->subtype >= ESP_PARTITION_SUBTYPE_APP_OTA_0 && p->subtype <= ESP_PARTITION_SUBTYPE_APP_OTA_15 &&
        p->address != running->address && strcmp(p->label, "recovery_app") != 0;
}
bool erasable(const esp_partition_t* p) {
    // Never erase apps, OTA selection, keys, calibration or unknown partitions.
    return p && p->type == ESP_PARTITION_TYPE_DATA && !p->readonly &&
        (p->subtype == ESP_PARTITION_SUBTYPE_DATA_SPIFFS ||
         p->subtype == ESP_PARTITION_SUBTYPE_DATA_FAT ||
         p->subtype == ESP_PARTITION_SUBTYPE_DATA_NVS ||
         p->subtype == ESP_PARTITION_SUBTYPE_DATA_COREDUMP);
}
}
