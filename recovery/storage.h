#pragma once
#include <esp_err.h>
#include <esp_partition.h>
#include <string>
namespace recovery {
bool spiffsMounted();
bool sdMounted();
esp_err_t mountSpiffs();
esp_err_t unmountSpiffs();
esp_err_t mountSD();
esp_err_t unmountSD();
std::string storageInfo(bool sd);
bool erasable(const esp_partition_t* partition);
esp_err_t eraseData(const esp_partition_t* partition);
}
