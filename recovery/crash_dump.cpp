#include "crash_dump.h"
#include "dump_export.h"
#include "storage.h"
#include <esp_core_dump.h>
#include <esp_partition.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
namespace recovery {
esp_err_t crashStatus(size_t& size) {
    size = 0;
#if CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH && CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF
    size_t address = 0;
    esp_err_t err = esp_core_dump_image_get(&address, &size);
    if (err != ESP_OK) return err;
    auto p = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_COREDUMP, nullptr);
    // SDK image_get only checks a four-byte minimum; enforce a full header and
    // SHA256 before calling its checksum parser to avoid size underflow.
    if (!p || address != p->address || size < 56 || size > p->size) return ESP_ERR_INVALID_SIZE;
    return esp_core_dump_image_check();
#else
    return ESP_ERR_NOT_SUPPORTED;
#endif
}
std::vector<std::string> crashSummary() {
    size_t size;
    esp_err_t err = crashStatus(size);
    if (err != ESP_OK) return {std::string("Saved dump: ") + esp_err_to_name(err)};
    std::vector<std::string> lines = {"Saved crash (may precede latest reset)", "Raw dump bytes: " + std::to_string(size)};
#if CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH && CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF
    char reason[192] = {};
    if (esp_core_dump_get_panic_reason(reason, sizeof(reason)) == ESP_OK)
        lines.push_back("Panic: " + std::string(reason, strnlen(reason, sizeof(reason))));
    esp_core_dump_summary_t summary = {};
    err = esp_core_dump_get_summary(&summary);
    if (err != ESP_OK) { lines.push_back(std::string("Summary: ") + esp_err_to_name(err)); return lines; }
    lines.push_back("Task: " + std::string(summary.exc_task, strnlen(summary.exc_task, sizeof(summary.exc_task))));
    char detail[96];
    snprintf(detail, sizeof(detail), "PC: 0x%08lx TCB: 0x%08lx", (unsigned long)summary.exc_pc, (unsigned long)summary.exc_tcb);
    lines.push_back(detail);
    snprintf(detail, sizeof(detail), "EXCCAUSE: %lu EXCVADDR: 0x%08lx", (unsigned long)summary.ex_info.exc_cause, (unsigned long)summary.ex_info.exc_vaddr);
    lines.push_back(detail);
    const char* sha = reinterpret_cast<const char*>(summary.app_elf_sha256);
    lines.push_back("ELF SHA256: " + std::string(sha, strnlen(sha, sizeof(summary.app_elf_sha256))));
    lines.push_back(summary.exc_bt_info.corrupted ? "Backtrace (corrupt/incomplete):" : "Backtrace addresses:");
    for (unsigned i = 0; i < std::min(summary.exc_bt_info.depth, uint32_t(16)); ++i) {
        snprintf(detail, sizeof(detail), "#%u 0x%08lx", i, (unsigned long)summary.exc_bt_info.bt[i]);
        lines.push_back(detail);
    }
#endif
    return lines;
}
bool saveCrash(bool sd, std::string& report) {
    size_t size;
    esp_err_t err = crashStatus(size);
    if (err != ESP_OK) { report = esp_err_to_name(err); return false; }
    err = sd ? mountSD() : mountSpiffs();
    if (err != ESP_OK) { report = std::string("Mount: ") + esp_err_to_name(err); return false; }
    auto partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_COREDUMP, nullptr);
    std::string base;
    for (unsigned index = 0; index < 10000; ++index) {
        char candidate[40];
        snprintf(candidate, sizeof(candidate), "%s/crsh%04u", sd ? "/sd" : "/spiffs", index);
        struct stat st;
        if (stat((std::string(candidate) + ".bin").c_str(), &st) != 0 &&
            stat((std::string(candidate) + ".txt").c_str(), &st) != 0 &&
            stat((std::string(candidate) + ".tmp").c_str(), &st) != 0) { base = candidate; break; }
    }
    if (base.empty()) { report = "No unused crash filename"; return false; }
    auto pause = [] { vTaskDelay(1); };
    if (!exportDump(base + ".bin", size, [partition](size_t offset, void* data, size_t n) {
        return esp_partition_read(partition, offset, data, n) == ESP_OK;
    }, report, pause)) return false;
    std::string summary;
    for (const auto& line : crashSummary()) summary += line + "\n";
    summary += "Format: raw ESP-IDF flash dump, ELF payload, SHA256 checksum\n";
    if (!exportDump(base + ".txt", summary.size(), [&summary](size_t offset, void* data, size_t n) {
        memcpy(data, summary.data() + offset, n); return true;
    }, report, pause)) {
        report = "Raw dump saved to " + base + ".bin; summary failed: " + report;
        return false;
    }
    report = "Saved " + base + ".bin and .txt. Flash dump retained.";
    return true;
}
esp_err_t clearCrash() { return esp_core_dump_image_erase(); }
}
