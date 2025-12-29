#include "ImageProcessor.h"
#define STBIR_MALLOC(size, user_data) ps_malloc(size)
#define STBIR_FREE(ptr, user_data)    free(ptr)

#define STBI_MALLOC(sz)        ps_malloc(sz)
#define STBI_REALLOC(p, newsz) ps_realloc(p, newsz)
#define STBI_FREE(p)           free(p)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

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

image_data displayPNG(const NString path, int w, int h,bool onlyParams) {

    IFile* f = VFS.open(path, "r");
    if (!f) {
        ESP_LOGE("IMG", "Failed to open %s", path.c_str());
        return {-1, -1, "Failed to open file"};
    }

    int            width, height, channels;
    unsigned char* img = loadPNGFromFile(f, &width, &height, &channels);
    f->close();

    if (!img) {
        ESP_LOGE("IMG", "Failed to decode %s", path.c_str());
        if (stbi_failure_reason()) { ESP_LOGE("IMG", "Reason: %s", stbi_failure_reason()); }
        else { ESP_LOGE("IMG", "Reason: unknown error"); }
        return {-1, -1, stbi_failure_reason(), nullptr};
    }    
    if(onlyParams){
        stbi_image_free(img);
        return {width,height,nullptr,nullptr};
    }

    int            targetW = (w > 0) ? w : width;
    int            targetH = (h > 0) ? h : height;
    unsigned char* resized = resizePNG(img, width, height, targetW, targetH);
    if (!resized) { return {-1, -1, "Failed to resize image"}; }
    uint16_t* data = convertRGBToRGB565(resized, targetW, targetH);

    return {width, height, nullptr, data};
}