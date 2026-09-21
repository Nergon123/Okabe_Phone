idf_component_register(SRCS "recovery_main.cpp" "storage.cpp" "image_check.cpp" "partition_policy.cpp" "crash_dump.cpp" "dump_export.cpp"
    REQUIRES espcoredump okabe_drivers op freertos log app_update esp_partition esp_system
             esp_app_format bootloader_support spiffs fatfs sdmmc driver mbedtls)
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
