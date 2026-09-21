#pragma once
#include <esp_err.h>
#include <string>
#include <vector>
namespace recovery {
esp_err_t crashStatus(size_t& size);
std::vector<std::string> crashSummary();
bool saveCrash(bool sd, std::string& report);
esp_err_t clearCrash();
}
