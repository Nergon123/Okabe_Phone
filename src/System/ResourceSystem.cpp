#include "ResourceSystem.h"
#include "../Platform/ESP32Memory.h"
#include "Generic.h"
Coords czero = {0, 0};
Coords cnone = {-1, -1};

void ResourceSystem::Init(NFile *Main, NFile *Wallpapers) {
    Files[RES_MAIN]       = Main;
    Files[RES_WALLPAPERS] = Wallpapers;
    Images[RES_MAIN].clear();
    Images[RES_WALLPAPERS].clear();
    for (int i = 0; i < sizeof(Files) / sizeof(Files[0]); i++) {
        if (!Files[i]) {
            ESP_LOGW("RES", "%s resource file is not available. Using Built-in.", names[i]);
        }
        else { parseResourceFile(Files[i], Headers[i], i, i == RES_MAIN); }
    }
};

void ResourceSystem::parseResourceFile(NFile *file, Header &header, uint8_t type, bool important) {
    Images[type].clear();
    if (file->read(reinterpret_cast<uint8_t *>(&header), sizeof(Header)) != sizeof(Header)) {
        failure("Error when reading Main Header", important);
        return;
    };
    if (strcmp(header.MAGIC, "NerPh") != 0) {
        failure("Header Mismatch (Wrong file?)", important);
        file = nullptr;
        return;
    }
    if (header.version != FILE_VERSION) {
        failure("File version mismatch.", important);
        file = nullptr;
        return;
    }
    if (header.imageCount == 0) {
        failure("There is no Images...", important);
        file = nullptr;
        return;
    }
    for (int i = 0; i < header.imageCount; ++i) {
        ImageData img;
        if (file->read(reinterpret_cast<uint8_t *>(&img), sizeof(ImageData)) !=
            sizeof(ImageData)) {
            failure("Error reading ImageData", important);
            return;
        }
        ESP_LOGV("RES",
                 "Parsed image %d: ID: %d, Count: %d, X: %d, Y: %d, Width: %d, Height: %d, "
                 "Offset: %d\n",
                 i, img.id, img.count, img.x, img.y, img.width, img.height, img.offset);
        Images[type].push_back(img);
    }
}

void ResourceSystem::failure(const char *msg, bool important) {
    ESP_LOGE("RES", "%s", msg);
    if (important) { sysError(msg); }
}

ImageData ResourceSystem::GetImageDataByID(uint16_t id, uint8_t type) {
    for (ImageData img : Images[type]) {
        if (img.id == id) { return img; }
    }
    return ImageData(R_NULL_IMAGE);
}

ImageData ResourceSystem::GetImageDataByImage(Image image) {
    for (ImageData img : Images[image.resType]) {
        if (img.id == image.id) { return img; }
    }
    return ImageData(R_NULL_IMAGE);
}

bool ResourceSystem::DrawImage(uint16_t id, uint8_t index, Coords pos, Coords startpos,
                               Coords endpos, uint8_t type, bool is_buffer, RenderTarget *target) {
    ImageData img = GetImageDataByID(id, type);
    if (img.id == R_NULL_IMAGE && id != R_NULL_IMAGE) {
        ESP_LOGE("RES", "Requested Sprite %d:%d not found", id, index);
        return false;
    }
    else if (id == R_NULL_IMAGE) { return false; }

    if (pos.x == OP_UNDEF) { pos.x = img.x; }
    if (pos.y == OP_UNDEF) { pos.y = img.y; }
    if (endpos.x <= 0) { endpos.x = img.width; }
    if (endpos.y <= 0) { endpos.y = img.height; }
    if (startpos.y < 0) { startpos.y = 0; }
    if (startpos.x < 0) { startpos.x = 0; }
    if (endpos.y > img.height) { endpos.y = img.height; }
    if (endpos.x > img.width) { endpos.x = img.width; }
    if (startpos.y >= endpos.y) { return false; }

    int16_t  width = endpos.x - startpos.x, height = endpos.y - startpos.y;
    uint32_t start = (startpos.y * img.width * 2 + (img.width * img.height * 2 * index));
    uint32_t size  = height * img.width * 2;
    if (size == 0) { return false; }

    bool isLines = psramFound() && !is_buffer;
    int  lines   = isLines ? img.height : lines_to_draw_wo_psram;
    ESP_LOGI("RES", "Drawing Image ID:%d Index:%d at %d,%d Size:%dx%d (from %d) on %s", id, index,
             pos.x, pos.y, width, height, start, is_buffer ? "TFT" : "RenderTarget");
    tft.setRenderTarget(target);
    for (int i = 0; i < height; i += lines) {

        if (height - i < lines) { lines = height % lines; }

        ImageBuffer imageBuffer =
            GetRGB565(img, lines * img.width * 2, start + i * img.width * 2, type);

        if (img.flags & 1 /*if transparent*/) {
            tft.pushImage(pos.x, pos.y + i, width, lines, imageBuffer.pointer, img.transpColor);
        }
        else { tft.pushImage(pos.x, pos.y + i, width, lines, imageBuffer.pointer); }
    }
    return true;
}
bool ResourceSystem::DrawImage(Image image, uint8_t index, Coords pos, Coords startpos,
                               Coords endpos, uint8_t type, bool is_tft, RenderTarget *target) {
    return DrawImage(image.id, index, pos, startpos, endpos, image.resType, is_tft, target);
}

bool ResourceSystem::DrawImage(uint16_t id, uint8_t index, bool is_tft, RenderTarget *target) {
    return DrawImage(id, index, {OP_UNDEF, OP_UNDEF}, {0, 0}, {-1, -1}, RES_MAIN, is_tft, target);
}

ImageBuffer ResourceSystem::GetRGB565(ImageData img, size_t size, uint32_t start, uint8_t type) {

    ImageBuffer buffer;
    if (cache[type]) {
        buffer.pointer    = reinterpret_cast<uint16_t *>(cache[type] + img.offset + start);
        buffer.freeNeeded = false;
        return buffer;
    }

    buffer.freeNeeded = true;
    if (psramFound()) { buffer.pointer = (uint16_t *)ps_malloc(size); }
    else { buffer.pointer = (uint16_t *)malloc(size); }

    Files[type]->seek(img.offset + start);
    Files[type]->read(reinterpret_cast<uint8_t *>(buffer.pointer), size);

    return buffer;
}

void ResourceSystem::CopyToRam(uint8_t type) {
    bootText("Copying file to RAM...");
    if (cache[type]) { free(cache[type]); }
    cache[type] = nullptr;
#ifndef PC
    if (psramFound() &&
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT) > Files[type]->size()) {
#else
    if (true) {
#endif
        Files[type]->seek(0);
        cache[type] = (uint8_t *)ps_malloc(Files[type]->size());

        if (cache[type]) {
            size_t readB =
                Files[type]->read(reinterpret_cast<uint8_t *>(cache[type]), Files[type]->size());

            if (readB != Files[type]->size()) {
                ESP_LOGE("CopyToRam", "Size mismatch %d != %d", Files[type]->size(), readB);
                bootText("copying to ram not successfull.");

               // free(cache[type]);
               // cache[type] = nullptr;
            }
        }
    }
}

ResourceSystem res;

void drawWallpaper() { res.DrawImage(R_DEFAULT_WALLPAPER); }