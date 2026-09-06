#include "ImageProcessor.h"
#define STBIR_MALLOC(size, user_data) ps_malloc(size)
#define STBIR_FREE(ptr, user_data)    free(ptr)

#define STBI_MALLOC(sz)        ps_malloc(sz)
#define STBI_REALLOC(p, newsz) ps_realloc(p, newsz)
#define STBI_FREE(p)           free(p)
#define STB_IMAGE_IMPLEMENTATION
#include <ExternalLibraries/stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <ExternalLibraries/stb_image_resize2.h>

// That's a lot of memory stuff, heavy for ESP32, even with PSRAM...
// works, but need to replace stb image lib with something lighter later

uint16_t* resizeRGB565buffer(uint16_t* buffer, int inputW, int inputH, int targetW, int targetH) {
    if (targetW == inputW && targetH == inputH) {
        return buffer; // no resize needed
    }

    uint8_t* tempRGB = (uint8_t*)ps_malloc(inputW * inputH * 3);
    for (int j = 0; j < inputH; j++) {
        for (int i = 0; i < inputW; i++) {
            int      idx        = (j * inputW + i);
            uint16_t pixel      = buffer[idx] >> 8 | (buffer[idx] & 0xFF)
                                                         << 8; // convert back to RGB565
            uint8_t  r          = ((pixel >> 11) & 0x1F) << 3;
            uint8_t  g          = ((pixel >> 5) & 0x3F) << 2;
            uint8_t  b          = (pixel & 0x1F) << 3;
            int      rgbIdx     = idx * 3;
            tempRGB[rgbIdx]     = r;
            tempRGB[rgbIdx + 1] = g;
            tempRGB[rgbIdx + 2] = b;
        }
    }

    uint8_t* resizedRGB = stbir_resize_uint8_srgb(tempRGB, inputW, inputH, 0, NULL, targetW,
                                                  targetH, 0, (stbir_pixel_layout)3);
    free(tempRGB);
    if (!resizedRGB) {
        free(buffer);
        return nullptr;
    }

    uint16_t* resizedRGB565 = (uint16_t*)ps_malloc(targetW * targetH * sizeof(uint16_t));
    for (int j = 0; j < targetH; j++) {
        for (int i = 0; i < targetW; i++) {
            int     idx        = (j * targetW + i);
            int     rgbIdx     = idx * 3;
            uint8_t r          = resizedRGB[rgbIdx];
            uint8_t g          = resizedRGB[rgbIdx + 1];
            uint8_t b          = resizedRGB[rgbIdx + 2];
            resizedRGB565[idx] = tft.color565(r, g, b);
        }
    }
    free(resizedRGB);
    return resizedRGB565;
}

uint8_t* resizePNG(uint8_t* img, int width, int height, int targetW, int targetH) {
    if (targetW == width && targetH == height) {
        return img; // no resize needed
    }

    uint8_t* resized = stbir_resize_uint8_srgb(img, width, height, 0, NULL, targetW, targetH, 0,
                                               (stbir_pixel_layout)3);
    stbi_image_free(img); // free original
    return resized;
}

uint8_t* loadPNGFromFile(NFile* file, int* width, int* height, int* channels) {
    if (!file) { return nullptr; }
    size_t   sz     = file->size();
    uint8_t* buffer = (uint8_t*)ps_malloc(sz);
    if (!buffer) { return nullptr; }

    file->read(buffer, sz);
    uint8_t* img = stbi_load_from_memory(buffer, sz, width, height, channels, 3);
    free(buffer);
    return img; // Remember to stbi_image_free(img) later
}
uint16_t* convertRGBToRGB565(uint8_t* img, int w, int h) {
    uint16_t* rgb565 = (uint16_t*)ps_malloc(w * h * sizeof(uint16_t));
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            int idx           = (j * w + i) * 3;
            rgb565[w * j + i] = tft.color565(img[idx], img[idx + 1], img[idx + 2]);
        }
    }
    return rgb565;
}

image_data displayPNG(const NString path, int w, int h, bool onlyParams) {

    IFile* f = VFS.open(path, "r");
    if (!f) {
        ESP_LOGE("IMG", "Failed to open %s", path.c_str());
        return {-1, -1, "Failed to open file", nullptr};
    }
    printf("Loading image from file: %s\n", path.c_str());

    int            width, height, channels;
    unsigned char* img = loadPNGFromFile(f, &width, &height, &channels);
    f->close();

    if (!img) {
        ESP_LOGE("IMG", "Failed to decode %s", path.c_str());
        if (stbi_failure_reason()) { ESP_LOGE("IMG", "Reason: %s", stbi_failure_reason()); }
        else {
            ESP_LOGE("IMG", "Reason: unknown error");
            return {-1, -1, "Unknown Error", nullptr};
        }
        return {-1, -1, stbi_failure_reason(), nullptr};
    }
    if (onlyParams) {
        stbi_image_free(img);
        return {width, height, nullptr, nullptr};
    }

    int            targetW = (w > 0) ? w : width;
    int            targetH = (h > 0) ? h : height;
    unsigned char* resized = resizePNG(img, width, height, targetW, targetH);
    if (!resized) { return {-1, -1, "Failed to resize image", nullptr}; }
    uint16_t* data = convertRGBToRGB565(resized, targetW, targetH);

    return {targetW, targetH, nullptr, data};
}