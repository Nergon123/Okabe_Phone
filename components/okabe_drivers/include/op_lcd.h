#pragma once
#include <esp_err.h>
#include <stdint.h>
namespace okabe {
esp_err_t initLCD();
// Input uses the application's byte-swapped RGB565 image format. Synchronous:
// the caller may reuse pixels immediately after this function returns.
// stride is the source row width in pixels; zero means tightly packed.
esp_err_t drawLCD(int x, int y, int width, int height, const uint16_t* pixels, int stride = 0);
}
