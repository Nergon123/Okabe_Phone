idf_component_register(SRCS "peripherals.cpp" "lcd.cpp" "keypad.cpp" INCLUDE_DIRS "include"
    REQUIRES driver esp_lcd esp_timer freertos)
if(IDF_TARGET STREQUAL "esp32s3")
    target_compile_definitions(${COMPONENT_LIB} PUBLIC OKABE_S3=1)
endif()
