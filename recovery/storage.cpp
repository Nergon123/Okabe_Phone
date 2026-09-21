#include "storage.h"
#include <esp_core_dump.h>
#include "partition_policy.h"
#include <op_board.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <driver/sdspi_host.h>
#include <esp_spiffs.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <cstdio>
#include <cstring>
namespace recovery {
namespace {
bool spiffs = false;
sdmmc_card_t* card = nullptr;
bool validSDPin(int pin, bool output) {
    if (!(output ? GPIO_IS_VALID_OUTPUT_GPIO(pin) : GPIO_IS_VALID_GPIO(pin))) return false;
#ifdef CONFIG_IDF_TARGET_ESP32S3
    if (pin >= 26 && pin <= 32) return false;
#ifdef CONFIG_SPIRAM_MODE_OCT
    if (pin >= 33 && pin <= 37) return false;
#endif
#endif
    const int used[] = {TFT_CS, TFT_DC, TFT_RST, TFT_BL, TFT_MOSI, TFT_MISO, TFT_SCLK,
                       I2C_SDA, I2C_SCL};
    for (int reserved : used) if (pin == reserved) return false;
    return true;
}
}
bool spiffsMounted() { return spiffs; }
bool sdMounted() { return card != nullptr; }
esp_err_t mountSpiffs() {
    if (spiffs) return ESP_OK;
    esp_vfs_spiffs_conf_t conf = {};
    conf.base_path = "/spiffs";
    conf.partition_label = "spiffs";
    conf.max_files = 4;
    conf.format_if_mount_failed = false;
    esp_err_t err = esp_vfs_spiffs_register(&conf);
    spiffs = err == ESP_OK;
    return err;
}
esp_err_t unmountSpiffs() {
    if (!spiffs) return ESP_OK;
    esp_err_t err = esp_vfs_spiffs_unregister("spiffs");
    if (err == ESP_OK) spiffs = false;
    return err;
}
esp_err_t mountSD() {
    if (card) return ESP_OK;
    if (SD_CS < 0 || SD_SCK < 0 || SD_MOSI < 0 || SD_MISO < 0) return ESP_ERR_NOT_SUPPORTED;
    const int pins[] = {SD_CS, SD_SCK, SD_MOSI, SD_MISO};
    for (unsigned i = 0; i < 4; ++i) {
        if (!validSDPin(pins[i], i != 3)) return ESP_ERR_INVALID_ARG;
        for (unsigned j = 0; j < i; ++j) if (pins[i] == pins[j]) return ESP_ERR_INVALID_ARG;
    }
    spi_bus_config_t bus = {};
    bus.mosi_io_num = SD_MOSI;
    bus.miso_io_num = SD_MISO;
    bus.sclk_io_num = SD_SCK;
    bus.quadwp_io_num = bus.quadhd_io_num = -1;
    bus.max_transfer_sz = 4096;
    esp_err_t err = spi_bus_initialize(SPI3_HOST, &bus, SPI_DMA_CH_AUTO);
    if (err != ESP_OK) return err;
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI3_HOST;
    sdspi_device_config_t slot = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot.host_id = SPI3_HOST;
    slot.gpio_cs = static_cast<gpio_num_t>(SD_CS);
    esp_vfs_fat_sdmmc_mount_config_t config = {};
    config.max_files = 4;
    config.format_if_mount_failed = false;
    err = esp_vfs_fat_sdspi_mount("/sd", &host, &slot, &config, &card);
    if (err != ESP_OK) { card = nullptr; spi_bus_free(SPI3_HOST); }
    return err;
}
esp_err_t unmountSD() {
    if (!card) return ESP_OK;
    esp_err_t err = esp_vfs_fat_sdcard_unmount("/sd", card);
    if (err != ESP_OK) return err;
    card = nullptr;
    return spi_bus_free(SPI3_HOST);
}
std::string storageInfo(bool sd) {
    char buf[128];
    if (sd) {
        if (!card) return "SD not mounted";
        snprintf(buf, sizeof(buf), "SD capacity: %llu MiB", (unsigned long long)card->csd.capacity * card->csd.sector_size / (1024 * 1024));
    } else {
        size_t total = 0, used = 0;
        esp_err_t err = esp_spiffs_info("spiffs", &total, &used);
        if (err != ESP_OK) return esp_err_to_name(err);
        snprintf(buf, sizeof(buf), "SPIFFS: %u / %u KiB used", unsigned(used / 1024), unsigned(total / 1024));
    }
    return buf;
}
esp_err_t eraseData(const esp_partition_t* p) {
    if (!erasable(p)) return ESP_ERR_NOT_ALLOWED;
    if (p->subtype == ESP_PARTITION_SUBTYPE_DATA_COREDUMP) return esp_core_dump_image_erase();
    if (!strcmp(p->label, "spiffs")) {
        esp_err_t err = unmountSpiffs();
        if (err != ESP_OK) return err;
    }
    return esp_partition_erase_range(p, 0, p->size);
}
}
