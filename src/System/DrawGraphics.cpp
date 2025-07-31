#include "DrawGraphics.h"

// ## Downscale and draw image
//
// This function was used but was removed because of too long render time
// still can be used in future to scale down images(?)
// @param pos: address in the file
// @param pos_x: x position on the screen
// @param pos_y: y position on the screen
// @param size_x: width of the image
// @param size_y: height of the image
// @param scale: downscale factor
// @param file_path: path to the file on the SD card
void drawFromSdDownscale(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, int scale, String file_path) {

    // Open the file from the SD card
    File file = SD.open(file_path);
    if (!file.available()) {
        sysError("SD_CARD_NOT_AVAILABLE");
        return;
    }
    file.seek(pos);

    // Calculate downscaled dimensions
    const int downscaled_width  = size_x / scale;
    const int downscaled_height = size_y / scale;

    // Allocate buffer for the full image (16-bit color, 2 bytes per pixel)
    const int image_size   = size_x * size_y * 2;
    uint8_t  *image_buffer = (uint8_t *)malloc(image_size);
    if (!image_buffer) {
        sysError("MEMORY_ALLOCATION_FAILED");
        file.close();
        
        return;
    }

    // Read the entire image into the buffer
    if (file.read(image_buffer, image_size) != image_size) {
        sysError("FILE_READ_ERROR");
        free(image_buffer);
        file.close();
        
        return;
    }
    file.close();

    // Allocate buffer for the downscaled image
    uint16_t *downscaled_buffer = (uint16_t *)malloc(downscaled_width * downscaled_height * sizeof(uint16_t));
    if (!downscaled_buffer) {
        sysError("MEMORY_ALLOCATION_FAILED");
        free(image_buffer);
        
        return;
    }

    // Downscale the image
    for (int y = 0; y < downscaled_height; y++) {
        for (int x = 0; x < downscaled_width; x++) {
            // Calculate the position in the original image buffer
            int src_x     = x * scale;
            int src_y     = y * scale;
            int src_index = (src_y * size_x + src_x) * 2;

            // Combine two bytes into a 16-bit color value
            uint16_t pixel = (image_buffer[src_index] << 8) | image_buffer[src_index + 1];

            // Store the pixel in the downscaled buffer
            downscaled_buffer[y * downscaled_width + x] = pixel;
        }
    }
    Serial.printf("Y:%d\n", downscaled_height);
    // Push the downscaled image to the screen
    tft.pushImage(pos_x, pos_y, downscaled_width, downscaled_height, downscaled_buffer);

    // Free allocated memory
    free(image_buffer);
    free(downscaled_buffer);

 
    
}




/*
* Draw cutout from SD card
* @param image: SDImage object containing the image data
* @param cutout_x: x position of the cutout in the image
* @param cutout_y: y position of the cutout in the image
* @param cutout_width: width of the cutout
* @param cutout_height: height of the cutout
* @param display_x: x position on the display
* @param display_y: y position on the display
* @param file_path: path to the image file on the SD card
*/
void drawCutoutFromSd(SDImage image,
                      int cutout_x, int cutout_y,
                      int cutout_width, int cutout_height,
                      int display_x, int display_y,
                      String file_path) {
    

    // Open the file from the SD card
    File file = SD.open(file_path);
    if (!file || !file.available()) {
        sysError("FILE_NOT_AVAILABLE");
        return;
    }

    int image_width = image.w;

    // Calculate the starting offset for the cutout in the file
    uint32_t start_offset = image.address + (cutout_y * image_width + cutout_x) * 2;

    const int buffer_size = cutout_width * 2; // 2 bytes per pixel (16-bit color)
    uint8_t   buffer[buffer_size];

    for (int row = 0; row < cutout_height; row++) {
        // Calculate the offset for the current row
        uint32_t row_offset = start_offset + (row * image_width * 2);
        file.seek(row_offset);

        // Read the row of pixel data into the buffer
        int bytesRead = file.read(buffer, buffer_size);
        if (bytesRead != buffer_size) {
            Serial.println("Error reading row from SD card.");
            return;
        }
        tft.pushImage(display_x, display_y + row, cutout_width, 1, (uint16_t *)buffer);
    }
    file.close();
    
}





TFT_eSprite sprite = TFT_eSprite(&tft);

// ## Draw image from SD card
//  This function draws an image from the SD card to the screen or a sprite buffer.
//  @param pos: address in the file
//  @param pos_x: x position on the screen
//  @param pos_y: y position on the screen
//  @param size_x: width of the image
//  @param size_y: height of the image
//  @param is_screen_buffer: true if the image should be drawn to a sprite buffer
//  @param sbuffer: sprite buffer to draw the image to
//  @param file_path: path to the file on the SD card
//  @param transp: true if the image has transparent pixels
//  @param tc: transparent color (default is black)
void drawImage(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, bool is_screen_buffer, TFT_eSprite &sbuffer, String file_path, bool transp, uint16_t tc) {
    
    if (file_path != resPath || !resources) {
        ESP_LOGI("DRAWING", "resource path is different from default or resources not available");
        #ifndef PSRAM_ENABLED
        ESP_LOGW("DRAWING", "PSRAM not available");
        #endif
        File file = SD.open(file_path, FILE_READ);
        if (!file.available())
            sysError("FILE_NOT_AVAILABLE");
        file.seek(pos);
        if (!transp) {
            const int buffer_size = size_x * 2;
            uint8_t   buffer[buffer_size];

            for (int a = 0; a < size_y; a++) {
                file.read(buffer, buffer_size);

                tft.pushImage(pos_x, a + pos_y, size_x, 1, (uint16_t *)buffer);
            }
        } else {

            const int buffer_size = size_x * 2; // 2 bytes per pixel
            uint8_t   buffer[buffer_size];

            for (int a = 0; a < size_y; a++) {
                // Read a whole line (row) of pixels at once
                file.read(buffer, buffer_size);

                int draw_start = -1; // Initialize start of draw segment

                for (int i = 0; i < size_x; i++) {
                    // Reconstruct 16-bit color from two bytes
                    uint16_t wd = (buffer[2 * i] << 8) | buffer[2 * i + 1];

                    if (wd != tc) { // If the pixel is not transparent
                        if (draw_start == -1) {
                            draw_start = i; // Start new draw segment
                        }
                    } else { // Transparent pixel
                        if (draw_start != -1) {
                            // Render segment up to current pixel
                            tft.pushImage(pos_x + draw_start, pos_y + a, i - draw_start, 1, (uint16_t *)(&buffer[2 * draw_start]));
                            draw_start = -1; // Reset draw_start
                        }
                    }
                }

                // Handle case where last segment reaches the end of the row
                if (draw_start != -1) {
                    tft.pushImage(pos_x + draw_start, pos_y + a, size_x - draw_start, 1, (uint16_t *)(&buffer[2 * draw_start]));
                }
            }
        }
        file.close();
    } else {
        pos -= RESOURCE_ADDRESS;
        // Serial.printf("POSITION %d\n", pos);

        if (resources) {

            uint16_t *imgData = (uint16_t *)(resources + (pos & ~1));
            if (!transp) {

                if (is_screen_buffer)
                    sbuffer.pushImage(pos_x, pos_y, size_x, size_y, imgData);
                else
                    tft.pushImage(pos_x, pos_y, size_x, size_y, imgData);
            } else {

                if (!sprite.createSprite(size_x, size_y)) {
                    Serial.println("Sprite allocation failed!");
                    return;
                }

                sprite.pushImage(0, 0, size_x, size_y, imgData);

                if (is_screen_buffer)
                    sprite.pushToSprite(&sbuffer, pos_x, pos_y, tc);
                else
                    sprite.pushSprite(pos_x, pos_y, tc);
                    

                sprite.deleteSprite();
            }
        }
    }

    
}


void drawImage(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, String file_path, bool transp, uint16_t tc) {
    drawImage(pos, pos_x, pos_y, size_x, size_y, false, screen_buffer, file_path, transp, tc);
}

void drawImage(uint32_t pos, int pos_x, int pos_y, int size_x, int size_y, bool transp, uint16_t tc) {
    drawImage(pos, pos_x, pos_y, size_x, size_y, false, screen_buffer, resPath, transp, tc);
}

// ## Draw image from SD card to sprite buffer
// @param x: x position on the screen
// @param y: y position on the screen
// @param sprite: SDImage object containing the image data
// @param is_screen_buffer: true if the image should be drawn to a sprite buffer
// @param sbuffer: sprite buffer to draw the image to
void drawImage(int x, int y, SDImage sprite, bool is_screen_buffer, TFT_eSprite &sbuffer) {
    if (sprite.address != 0)
        drawImage(sprite.address, x, y, sprite.w, sprite.h, is_screen_buffer, sbuffer, resPath, sprite.transp, sprite.tc);
}

// ## Draw image from SD card to screen
// @param x: x position on the screen
// @param y: y position on the screen
// @param sprite: SDImage object containing the image data
void drawImage(int x, int y, SDImage sprite) {
    if (sprite.address != 0)
        drawImage(sprite.address, x, y, sprite.w, sprite.h, sprite.transp, sprite.tc);
}

uint8_t *wallpaper = nullptr;
// draw wallpaper from buffer
void     drawWallpaper() {
    if (!isSPIFFS && SD.exists(resPath)) {
        if (!wallpaper) {
            if (wallpaperIndex >= 0 && wallpaperIndex < 42) {
                loadResource(WALLPAPER_IMAGES_BASE.address + WALLPAPER_IMAGES_MULTIPLIER * wallpaperIndex, resPath, &wallpaper, WALLPAPER_IMAGES_BASE.w, WALLPAPER_IMAGES_BASE.h);
            } else {
                if (SD.exists(currentWallpaperPath)) {
                    drawPNG(currentWallpaperPath.c_str(), true);
                } else {
                    wallpaperIndex = 0;
                    loadResource(WALLPAPER_IMAGES_BASE.address + WALLPAPER_IMAGES_MULTIPLIER * wallpaperIndex, resPath, &wallpaper, WALLPAPER_IMAGES_BASE.w, WALLPAPER_IMAGES_BASE.h);
                }
            }
            tft.pushImage(0, 26, 240, 294, (uint16_t *)wallpaper);
        } else {
            tft.pushImage(0, 26, 240, 294, (uint16_t *)wallpaper);
            Serial.println("WALLPAPER");
        }
    } else {
        if (SD.exists(currentWallpaperPath)) {
            drawPNG(currentWallpaperPath.c_str(), true);
        } else {
            uint16_t colora = random(0, 0x7777);
            uint16_t colorb = random(0, 0x7777);
            if (random(0, 1000) % 2 == 0)
                tft.fillRectVGradient(0, 26, 240, 294, colora, colorb);
            else
                tft.fillRectHGradient(0, 26, 240, 294, colora, colorb);
            changeFont(0);
            tft.setTextSize(1);
            tft.setTextColor(~colora);
            tft.setCursor(0, 26);
            tft.println("NO SDCARD OR RESOURCE FILE.\nBUILT-IN WALLPAPERS WILL NOT BE VISIBLE");
        }
    }
}
