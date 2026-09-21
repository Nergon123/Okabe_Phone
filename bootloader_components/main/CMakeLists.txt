idf_component_register(
    SRCS "bootloader_start.c"
    INCLUDE_DIRS "."
    # Reset-reason enum only; do not link application esp_system into bootloader.
    PRIV_INCLUDE_DIRS "${IDF_PATH}/components/esp_system/include"
)