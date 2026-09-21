#include "image_check.h"
 #include "crash_dump.h"
#include "storage.h"
#include "partition_policy.h"
#include <Fonts/font5x7.h>
#include <op_keypad.h>
#include <op_lcd.h>
#include <op_peripherals.h>
#include <esp_app_desc.h>
#include <esp_chip_info.h>
#include <esp_image_format.h>
#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_spiffs.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <mbedtls/sha256.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <vector>

namespace {
constexpr char TAG[] = "RECOVERY";
constexpr int width = 240, height = 320;
constexpr int keyUp = 2, keyDown = 8, keySelect = 5, keyBack = 9;
uint16_t band[width * 8];
unsigned char ioBuffer[4096];
using Lines = std::vector<std::string>;

void text(int y, const std::string& value, bool selected = false) {
    std::fill(std::begin(band), std::end(band), selected ? 0xffff : 0);
    for (size_t c = 0; c < value.size() && c < 40; ++c) {
        unsigned ch = static_cast<unsigned char>(value[c]);
        for (int x = 0; x < 5; ++x)
            for (int bit = 0; bit < 8; ++bit)
                if (font[ch * 5 + x] & (1u << bit))
                    band[bit * width + c * 6 + x] = selected ? 0 : 0xffff;
    }
    ESP_ERROR_CHECK(okabe::drawLCD(0, y, width, 8, band));
}
void clear() {
    memset(band, 0, sizeof(band));
    for (int y = 0; y < height; y += 8)
        ESP_ERROR_CHECK(okabe::drawLCD(0, y, width, 8, band));
}
Lines wrap(const std::string& s) {
    Lines result;
    for (size_t i = 0; i < s.size() && result.size() < 9; i += 38)
        result.push_back(s.substr(i, 38));
    return result;
}
int readKey() {
    static uint8_t address = 0x20;
    static bool ready = false;
    static TickType_t retryAt = 0;
    static int previous = -1, stable = -1, samples = 0;
    for (;;) {
        if (!ready && static_cast<int32_t>(xTaskGetTickCount() - retryAt) >= 0) {
            if (okabe::initI2C() == ESP_OK) {
                for (unsigned a = 0x20; a <= 0x27; ++a) {
                    if (okabe::probeI2C(a) && okabe::initKeypad(a) == ESP_OK) {
                        address = a;
                        ready = true;
                        break;
                    }
                }
            }
            text(312, ready ? "Keypad connected" : "Keypad missing; retrying...");
            retryAt = xTaskGetTickCount() + pdMS_TO_TICKS(2000);
        }
        if (ready) {
            int key = 0;
            esp_err_t err = okabe::scanKeypad(address, key);
            if (err != ESP_OK) {
                ESP_LOGW(TAG, "Keypad: %s", esp_err_to_name(err));
                ready = false;
                previous = stable = -1;
                samples = 0;
                retryAt = xTaskGetTickCount() + pdMS_TO_TICKS(2000);
                text(312, "Keypad lost; retrying...");
            } else {
                samples = key == previous ? std::min(samples + 1, 3) : 1;
                previous = key;
                if (samples == 3 && key != stable) {
                    stable = key;
                    if (key) return key;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
int menu(const std::string& title, const Lines& items, const Lines& info = {}) {
    int selected = 0;
    for (;;) {
        clear();
        text(0, title);
        int y = 20;
        size_t intro = std::min(info.size(), size_t(12));
        for (size_t i = 0; i < intro; ++i, y += 12) text(y, info[i]);
        if (intro) y += 8;
        int pageSize = std::max(1, (280 - y) / 12);
        int first = selected / pageSize * pageSize;
        for (int i = first; i < int(items.size()) && i < first + pageSize; ++i, y += 12)
            text(y, (i == selected ? "> " : "  ") + items[i], i == selected);
        text(288, "UP/DOWN move | OK select | BACK exit");
        text(300, std::to_string(selected + 1) + "/" + std::to_string(items.size()));
        for (;;) {
            int key = readKey();
            if (key == keyBack || key == 4) return -1;
            if (key == keySelect && !items.empty()) return selected;
            if (key == keyUp && selected > 0) { --selected; break; }
            if (key == keyDown && selected + 1 < int(items.size())) { ++selected; break; }
        }
    }
}
void message(const std::string& title, const std::string& body) {
    ESP_LOGI(TAG, "%s: %s", title.c_str(), body.c_str());
    menu(title, {"Back"}, wrap(body));
}
void result(const std::string& title, esp_err_t err) {
    message(title, err == ESP_OK ? "Completed" : esp_err_to_name(err));
}
bool confirm(const std::string& title, const Lines& details) {
    return menu(title, {"Cancel", "Confirm"}, details) == 1;
}
void progress(const std::string& action, size_t done, size_t total) {
    char line[40];
    snprintf(line, sizeof(line), "%s %u%%", action.c_str(), unsigned(done * 100 / std::max(total, size_t(1))));
    text(260, line);
}
bool shaCheck(FILE* file, const recovery::ImageInfo& info) {
    if (!info.hashOffset) return true;
    rewind(file);
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    bool ok = mbedtls_sha256_starts(&ctx, 0) == 0;
    size_t remaining = info.hashOffset;
    while (ok && remaining) {
        size_t n = std::min(remaining, sizeof(ioBuffer));
        ok = fread(ioBuffer, 1, n, file) == n && mbedtls_sha256_update(&ctx, ioBuffer, n) == 0;
        remaining -= n;
        vTaskDelay(1);
    }
    unsigned char actual[32], expected[32];
    ok = ok && mbedtls_sha256_finish(&ctx, actual) == 0 &&
        fread(expected, 1, sizeof(expected), file) == sizeof(expected) &&
        memcmp(actual, expected, sizeof(actual)) == 0;
    mbedtls_sha256_free(&ctx);
    rewind(file);
    return ok;
}
bool otaTarget(const esp_partition_t* p) {
    return recovery::canUpdate(p, esp_ota_get_running_partition());
}
std::vector<const esp_partition_t*> partitions() {
    std::vector<const esp_partition_t*> values;
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, nullptr);
    while (it) { values.push_back(esp_partition_get(it)); it = esp_partition_next(it); }
    return values;
}
void update(const std::string& path) {
    std::vector<const esp_partition_t*> targets;
    Lines labels;
    for (auto p : partitions()) if (otaTarget(p)) { targets.push_back(p); labels.push_back(p->label); }
    if (targets.empty()) { message("Update unavailable", "No non-running OTA app partition. Boot the factory recovery app first."); return; }
    int selected = menu("Update target", labels, wrap(path));
    if (selected < 0) return;
    const esp_partition_t* target = targets[selected];
    std::unique_ptr<FILE, decltype(&fclose)> file(fopen(path.c_str(), "rb"), fclose);
    if (!file) { message("Update failed", "Cannot open file"); return; }
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    recovery::ImageInfo image;
    std::string error;
    text(260, "Checking image...");
    if (!recovery::inspectImage(file.get(), target->size, CONFIG_IDF_FIRMWARE_CHIP_ID, chip.revision, image, error, [] { vTaskDelay(1); })) {
        message("Image rejected", error); return;
    }
    if (!shaCheck(file.get(), image)) { message("Image rejected", "SHA256 mismatch or read failure"); return; }
    Lines details = {std::string("Replace: ") + target->label,
                     "Bytes: " + std::to_string(image.size),
                     "Project: " + image.project, "Version: " + image.version,
                     "Old app in this slot will be lost.",
                     "Keep power connected until complete."};
    if (!confirm("Install firmware?", details)) return;
    clear(); text(0, "INSTALLING FIRMWARE");
    text(20, std::string("Target: ") + target->label);
    rewind(file.get());
    esp_ota_handle_t handle = 0;
    esp_err_t err = esp_ota_begin(target, image.size, &handle);
    if (err != ESP_OK) { result("Cannot start update", err); return; }
    size_t done = 0;
    while (done < image.size) {
        size_t n = std::min(image.size - done, sizeof(ioBuffer));
        if (fread(ioBuffer, 1, n, file.get()) != n) { err = ESP_FAIL; break; }
        err = esp_ota_write(handle, ioBuffer, n);
        if (err != ESP_OK) break;
        done += n;
        progress("Writing", done, image.size);
        vTaskDelay(1);
    }
    if (err != ESP_OK) {
        esp_ota_abort(handle);
        result("Update failed; stay in recovery", err);
        return;
    }
    text(260, "Verifying flash...");
    err = esp_ota_end(handle); // Includes IDF image/hash validation.
    if (err != ESP_OK) { result("Verification failed", err); return; }
    err = esp_ota_set_boot_partition(target);
    if (err != ESP_OK) { result("Boot selection failed", err); return; }
    if (menu("Update installed", {"Stay in recovery", "Reboot to main"}, {"Image verified; boot target saved."}) == 1)
        esp_restart();
}
struct Entry { std::string name; bool directory; size_t size; };
void browse(bool sd) {
    esp_err_t err = sd ? recovery::mountSD() : recovery::mountSpiffs();
    if (err != ESP_OK) { result("Mount failed", err); return; }
    const std::string root = sd ? "/sd" : "/spiffs";
    std::string path = root;
    for (;;) {
        DIR* dir = opendir(path.c_str());
        if (!dir) { message("Browse failed", "Cannot read directory"); return; }
        std::vector<Entry> entries;
        bool truncated = false;
        while (dirent* ent = readdir(dir)) {
            std::string name = ent->d_name;
            if (name == "." || name == "..") continue;
            // SPIFFS may expose slash-separated flat names; all remain under root.
            if (name.empty() || name[0] == '/' || name.find("../") != std::string::npos) continue;
            struct stat st;
            if (stat((path + "/" + name).c_str(), &st) != 0) continue;
            if (!S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode)) continue;
            if (entries.size() == 256) { truncated = true; break; }
            entries.push_back({name, S_ISDIR(st.st_mode), size_t(st.st_size)});
        }
        closedir(dir);
        std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
            if (a.directory != b.directory) return a.directory;
            return a.name < b.name;
        });
        Lines labels = {path == root ? "Back" : ".."};
        for (const auto& entry : entries) labels.push_back((entry.directory ? "[DIR] " : "") + entry.name);
        Lines details = wrap(path);
        if (truncated) details.push_back("First 256 entries shown");
        int selection = menu("FILES", labels, details);
        if (selection <= 0) {
            if (path == root) return;
            path.resize(path.find_last_of('/'));
            continue;
        }
        Entry entry = entries[selection - 1];
        std::string filePath = path + "/" + entry.name;
        if (entry.directory) { path = filePath; continue; }
        std::string extension = entry.name.substr(entry.name.find_last_of('.') == std::string::npos ? entry.name.size() : entry.name.find_last_of('.'));
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; });
        Lines info = wrap(filePath);
        info.push_back("Bytes: " + std::to_string(entry.size));
        int action = menu("FILE", extension == ".bin" ? Lines{"Back", "Install application .bin"} : Lines{"Back"}, info);
        if (action == 1 && extension == ".bin") update(filePath);
    }
}
void storageMenu(bool sd) {
    for (;;) {
        bool mounted = sd ? recovery::sdMounted() : recovery::spiffsMounted();
        Lines actions = {mounted ? "Unmount" : "Mount", "Browse files / update .bin", "Storage information"};
        if (!sd) { actions.push_back("Check SPIFFS"); actions.push_back("Format SPIFFS (erase files)"); }
        int action = menu(sd ? "SD CARD" : "SPIFFS", actions, {mounted ? "Mounted" : "Not mounted"});
        if (action < 0) return;
        if (action == 0) {
            esp_err_t err = sd ? (mounted ? recovery::unmountSD() : recovery::mountSD()) :
                                (mounted ? recovery::unmountSpiffs() : recovery::mountSpiffs());
            if (sd && err == ESP_ERR_NOT_SUPPORTED)
                message("SD not configured", "Set SD_CS, SD_SCK, SD_MOSI and SD_MISO to your wiring in platformio.ini.");
            else result(mounted ? "Unmount" : "Mount", err);
        } else if (action == 1) browse(sd);
        else if (action == 2) message("STORAGE", recovery::storageInfo(sd));
        else if (action == 3) {
            if (confirm("Check / repair SPIFFS?", {"The checker may repair filesystem data."})) {
                esp_err_t err = recovery::mountSpiffs();
                if (err == ESP_OK) err = esp_spiffs_check("spiffs");
                result("SPIFFS check", err);
            }
        } else if (action == 4 && confirm("FORMAT SPIFFS?", {"All files in spiffs will be lost.", "This cannot be undone."})) {
            esp_err_t err = recovery::unmountSpiffs();
            if (err == ESP_OK) err = esp_spiffs_format("spiffs");
            result("SPIFFS format", err);
        }
    }
}
void partitionMenu() {
    for (;;) {
        auto parts = partitions();
        Lines labels;
        const esp_partition_t* running = esp_ota_get_running_partition();
        const esp_partition_t* boot = esp_ota_get_boot_partition();
        for (auto p : parts) {
            char line[96];
            snprintf(line, sizeof(line), "%s %06lx %luK", p->label, (unsigned long)p->address, (unsigned long)p->size / 1024);
            labels.push_back(line);
            ESP_LOGI(TAG, "%s type=%02x subtype=%02x size=0x%lx encrypted=%d readonly=%d running=%d boot=%d", line,
                     p->type, p->subtype, (unsigned long)p->size, p->encrypted, p->readonly,
                     running && p->address == running->address, boot && p->address == boot->address);
        }
        int choice = menu("PARTITIONS: label / offset / KiB", labels);
        if (choice < 0) return;
        auto p = parts[choice];
        char location[64], types[64];
        snprintf(location, sizeof(location), "Offset 0x%08lx size 0x%08lx", (unsigned long)p->address, (unsigned long)p->size);
        snprintf(types, sizeof(types), "Type %02x subtype %02x enc=%d ro=%d", p->type, p->subtype, p->encrypted, p->readonly);
        Lines info = {p->label, location, types,
            running && p->address == running->address ? "Currently running" : "Not running",
            boot && p->address == boot->address ? "Configured boot target" : ""};
        if (p->type == ESP_PARTITION_TYPE_APP) {
            esp_app_desc_t desc;
            if (esp_ota_get_partition_description(p, &desc) == ESP_OK) {
                info.push_back("Project: " + std::string(desc.project_name, strnlen(desc.project_name, sizeof(desc.project_name))));
                info.push_back("Version: " + std::string(desc.version, strnlen(desc.version, sizeof(desc.version))));
            }
            Lines actions = {"Back", "Verify application"};
            bool selectable = otaTarget(p);
            if (selectable) actions.push_back("Boot this application");
            int action = menu("APP PARTITION", actions, info);
            if (action == 1) {
                esp_partition_pos_t pos = {p->address, p->size};
                esp_image_metadata_t metadata = {};
                result("Verify application", esp_image_verify(ESP_IMAGE_VERIFY, &pos, &metadata));
            } else if (action == 2 && selectable && confirm("BOOT APPLICATION?", {p->label, "Validate image, select and reboot."})) {
                esp_err_t err = esp_ota_set_boot_partition(p);
                if (err == ESP_OK) esp_restart();
                result("Cannot select app", err);
            }
        } else {
            bool allowed = recovery::erasable(p);
            int action = menu("DATA PARTITION", allowed ? Lines{"Back", "Erase entire data partition"} : Lines{"Back"}, info);
            if (action == 1 && allowed && confirm("ERASE DATA PARTITION?", {p->label, location, "All data here will be lost.", "This cannot be undone."})) {
                text(260, "Erasing...");
                result("Erase data", recovery::eraseData(p));
            }
        }
    }
}
void crashMenu() {
    for (;;) {
        size_t size = 0;
        esp_err_t status = recovery::crashStatus(size);
        Lines info = {status == ESP_OK ? "Valid saved crash: " + std::to_string(size) + " bytes" :
                      std::string("Dump status: ") + esp_err_to_name(status),
                      "First crash kept until explicitly cleared."};
        int action = menu("SAVED CORE DUMP", {"Show crash details", "Export to SPIFFS", "Export to SD", "Clear saved dump"}, info);
        if (action < 0) return;
        if (action == 0) {
            Lines display;
            for (const auto& line : recovery::crashSummary()) {
                ESP_LOGI(TAG, "%s", line.c_str());
                auto wrapped = wrap(line);
                display.insert(display.end(), wrapped.begin(), wrapped.end());
            }
            menu("CRASH DETAILS (scroll with UP/DOWN)", display);
        } else if (action == 1 || action == 2) {
            text(260, "Exporting and verifying...");
            std::string report;
            bool ok = recovery::saveCrash(action == 2, report);
            message(ok ? "Export complete" : "Export incomplete", report);
        } else if (action == 3 && confirm("CLEAR SAVED CRASH?", {"This permanently deletes the flash dump.", "Export it first if you need it.", "The next panic can then be captured."})) {
            result("Clear core dump", recovery::clearCrash());
        }
    }
}
const char* resetCause(esp_reset_reason_t reason) {
    switch (reason) {
    case ESP_RST_PANIC: return "Panic / unhandled exception";
    case ESP_RST_INT_WDT: return "Interrupt watchdog timeout";
    case ESP_RST_TASK_WDT: return "Task watchdog timeout";
    case ESP_RST_WDT: return "Hardware watchdog reset";
    case ESP_RST_BROWNOUT: return "Brownout / supply voltage drop";
    case ESP_RST_POWERON: return "Power on / external reset";
    case ESP_RST_SW: return "Requested software restart";
    case ESP_RST_DEEPSLEEP: return "Deep sleep wake";
    case ESP_RST_USB: return "USB reset";
    default: return "Other / unknown reset";
    }
}
bool abnormalReset(esp_reset_reason_t reason) {
    return reason == ESP_RST_PANIC || reason == ESP_RST_INT_WDT ||
           reason == ESP_RST_TASK_WDT || reason == ESP_RST_WDT ||
           reason == ESP_RST_BROWNOUT;
}
void resetScreen() {
    esp_reset_reason_t reason = esp_reset_reason();
    Lines info = {resetCause(reason), "Reset code: " + std::to_string(int(reason))};
    size_t dumpSize = 0;
    esp_err_t dumpStatus = recovery::crashStatus(dumpSize);
    info.push_back(dumpStatus == ESP_OK ? "Saved crash available in Core dump menu." :
                   "No valid saved dump: " + std::string(esp_err_to_name(dumpStatus)));
    info.push_back("A saved dump can be from an earlier crash.");
    if (dumpStatus == ESP_OK) {
        for (const auto& line : recovery::crashSummary()) {
            if (line.rfind("Panic:", 0) == 0 || line.rfind("Task:", 0) == 0 || line.rfind("PC:", 0) == 0) {
                for (const auto& chunk : wrap(line)) {
                    if (info.size() < 10) info.push_back(chunk);
                }
            }
        }
    }
    ESP_LOGW(TAG, "Previous reset: %s (%d)", resetCause(reason), int(reason));
    int action = menu(abnormalReset(reason) ? "GURU MEDITATION" : "LAST RESET",
                      {"Open recovery", "View saved core dump", "Retry normal boot"}, info);
    if (action == 1) crashMenu();
    else if (action == 2) esp_restart();
}
void run(void*) {
    ESP_ERROR_CHECK(okabe::initLCD());
    clear();
    ESP_ERROR_CHECK(okabe::initBacklight());
    okabe::setBacklight(100);
    if (abnormalReset(esp_reset_reason())) resetScreen();
    for (;;) {
        int action = menu("OKABE RECOVERY", {"SPIFFS", "SD card", "Partitions / boot / erase", "Last reset / diagnostics", "Saved core dump", "Reboot"},
                          {"Mount storage, select a .bin to update.", "App binaries only; no merged images."});
        if (action == 0) storageMenu(false);
        else if (action == 1) storageMenu(true);
        else if (action == 2) partitionMenu();
        else if (action == 3) resetScreen();
        else if (action == 4) crashMenu();
        else if (action == 5 && confirm("REBOOT?", {"Restart using configured boot selection."})) esp_restart();
    }
}
}
extern "C" void app_main() {
    ESP_LOGI(TAG, "Recovery starting");
    if (xTaskCreate(run, "recovery", 16384, nullptr, 5, nullptr) != pdPASS)
        ESP_LOGE(TAG, "Cannot allocate recovery task");
}
