#pragma once
#include <esp_partition.h>
namespace recovery {
bool canUpdate(const esp_partition_t* target, const esp_partition_t* running);
bool erasable(const esp_partition_t* partition);
}
