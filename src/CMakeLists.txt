file(GLOB_RECURSE app_sources "${CMAKE_CURRENT_LIST_DIR}/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/*.c")
idf_component_register(SRCS ${app_sources} INCLUDE_DIRS "."
    REQUIRES espcoredump okabe_drivers op freertos esp_timer esp_system esp_wifi esp_netif
             esp_event esp_http_client mbedtls nvs_flash spiffs fatfs sdmmc
             driver esp_driver_i2s esp_lcd esp_psram app_update)
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
# Existing bundled Lua/zlib and UI sources are not warning-clean yet.
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error)
