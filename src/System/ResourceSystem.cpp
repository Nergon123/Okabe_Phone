#include "ResourceSystem.h"
#include "Generic.h"
#include <System/LanguageSystem.h>
#include <System/Zip/ZipFileProvider.h>
Coords czero = {0, 0};
Coords cnone = {-1, -1};

void ResourceSystem::Init(NFile *Main, bool _important) {
    important = _important;
    Files     = Main;
    free(cache);
    cache = nullptr;
    Images.clear();
    ZipFileProvider *zfp = new ZipFileProvider();
    if (NString(Main->name()).endsWith(".npz")) {

        int res = zfp->openZip(Main);
        if (res) {
            ESP_LOGE("ZIP", "opening zip failed with code %d", res);
            zfp = nullptr;
            return;
        }
        res = zfp->setFile("file.nph");
        if (res) {
            ESP_LOGE("ZIP", "setting file failed with code %d", res);
            zfp = nullptr;
            return;
        }
        Files = zfp;
    }

    if (!Files) { ESP_LOGW("RES", "resource file is not available. Using Built-in."); }
    else { parseResourceFile(Files, Headers, important); }
    if (zfp) { zfp->setFile("file.nph"); }
};

void ResourceSystem::parseResourceFile(NFile *file, Header &header, bool important) {
    Images.clear();
    if (file->read(reinterpret_cast<uint8_t *>(&header), sizeof(Header)) != sizeof(Header)) {
        failure(getTranslation(TextKey::RES_FAIL_FILE_HEADER), important);
        return;
    };
    if (strcmp(header.MAGIC, "NerPh") != 0) {
        failure(getTranslation(TextKey::RES_FAIL_MAGIC_MISMATCH), important);
        file = nullptr;
        return;
    }
    if (header.version != FILE_VERSION) {
        failure(getTranslation(TextKey::RES_FAIL_VERSION_MISMATCH), important);
        file = nullptr;
        return;
    }
    if (header.imageCount == 0) {
        failure(getTranslation(TextKey::RES_FAIL_NO_IMAGES), important);
        file = nullptr;
        return;
    }
    for (int i = 0; i < header.imageCount; ++i) {
        ImageData img;
        if (file->read(reinterpret_cast<uint8_t *>(&img), sizeof(ImageData)) !=
            sizeof(ImageData)) {
            failure(getTranslation(TextKey::RES_FAIL_IMGDATA), important);
            return;
        }
        ESP_LOGV("RES",
                 "Parsed image %d: ID: %d, Count: %d, X: %d, Y: %d, Width: %d, Height: %d, "
                 "Offset: %d\n",
                 i, img.id, img.count, img.x, img.y, img.width, img.height, img.offset);
        Images.push_back(img);
    }
}

void ResourceSystem::failure(NString msg, bool important) {
    ESP_LOGE("RES", "%s", msg.c_str());
    if (important) { sysError(msg); }
}

ImageData ResourceSystem::GetImageDataByID(uint16_t id) {
    if (Images.empty()) { return ImageData(R_NULL_IMAGE); }
    for (ImageData &img : Images) {
        if (img.id == id) { return img; }
    }
    return ImageData(R_NULL_IMAGE);
}

ImageData ResourceSystem::GetImageDataByImage(Image image) {
    if (image.w) {
        ImageData img;
        img.id     = R_NULL_IMAGE;
        img.width  = image.w;
        img.height = image.h;
        return img;
    }
    if (Images.empty()) {
        ESP_LOGE("RES", "Images are empty!!!!");
        return R_NULL_IMAGE;
    }
    for (ImageData img : Images) {
        if (img.id == image.id) { return img; }
    }

    return ImageData(R_NULL_IMAGE);
}

bool ResourceSystem::DrawImage(uint16_t id, uint8_t index, Coords pos, Coords startpos,
                               Coords endpos) {
    ImageData img = GetImageDataByID(id);
    if (img.id == R_NULL_IMAGE && id != R_NULL_IMAGE) {
        ESP_LOGE("RES", "Requested Sprite %d:%d not found", id, index);
        return false;
    }
    else if (id == R_NULL_IMAGE) { return false; }
    if (index >= img.count) { return false; } // out of array
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
#ifndef PC
    bool is_buffer = tft.activeRenderTarget->getType() == RENDER_TARGET_TYPE_BUFFER;
    bool isLines   = psramFound() && !is_buffer;
#else
    bool isLines = false;
#endif
    int lines = isLines ? lines_to_draw_wo_psram : img.height;
    for (int i = 0; i < height; i += lines) {

        if (height - i < lines) { lines = height % lines; }

        ImageBuffer imageBuffer = GetRGB565(img, lines * img.width * 2, start + i * img.width * 2);
        if (!imageBuffer.pointer) {
            sysError(getTranslation(TextKey::RES_FAIL_NULL_IMGBUFFER));
            return false;
        }
        if (img.flags & 1 /*if transparent*/) {

            tft.pushImage(pos.x, pos.y + i, width, lines, imageBuffer.pointer, img.transpColor);
        }
        else { tft.pushImage(pos.x, pos.y + i, width, lines, imageBuffer.pointer); }
    }
    return true;
}
bool ResourceSystem::DrawImage(Image image, uint8_t index, Coords pos, Coords startpos,
                               Coords endpos) {

    if (image.buffer) { image.type = RES_POINTER; }
    else if (image.id != R_NULL_IMAGE) { image.type = RES_RESFILE; }
    else if (image.source) { image.type = RES_ADDRFILE; }
    else { image.type = RES_NULLU8; }
    switch (image.type) {
    case RES_NULLU8: return false;
    case RES_RESFILE: return DrawImage(image.id, index, pos, startpos, endpos);
    case RES_POINTER: tft.pushImage(pos.x, pos.y, image.w, image.h, image.buffer); return true;
    case RES_ADDRFILE: {
        ImageBuffer imgBuf =
            GetRGB565(GetImageDataByID(image.id), image.sw * image.sh * 2, image.id);
        if (!imgBuf.pointer) {
            sysError(getTranslation(TextKey::RES_FAIL_NULL_IMGBUFFER));
            return false;
        }
        tft.pushImage(pos.x, pos.y, image.w ? image.w : image.sw, image.h ? image.h : image.sh,
                      imgBuf.pointer);
        if (imgBuf.freeNeeded) { free(imgBuf.pointer); }
        return true;
    }
    default: return false;
    }
    return DrawImage(image.id, index, pos, startpos, endpos);
}

ImageBuffer ResourceSystem::GetRGB565(ImageData img, size_t size, uint32_t start) {

    ImageBuffer buffer;
    if (cache) {
        buffer.pointer    = reinterpret_cast<uint16_t *>(cache + img.offset + start);
        buffer.freeNeeded = false;
        return buffer;
    }

    buffer.freeNeeded = true;
    buffer.pointer    = (uint16_t *)malloc(size);

    Files->seek(img.offset + start, SEEK_SET);
    Files->read(reinterpret_cast<uint8_t *>(buffer.pointer), size);

    return buffer;
}

void ResourceSystem::CopyToRam(bool checksum) {
    bootText(getTranslation(TextKey::BOOT_CP_FILE_RAM));

    if (cache) { free(cache); }
    cache = nullptr;
#ifndef PC
    ESP_LOGI("PSRAM", "Free %u bytes from %u required",
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT), Files->size());
    if (psramFound() &&
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT) > Files->size())

#endif
    {
        Files->seek(0, SEEK_SET);
        cache = (uint8_t *)ps_malloc(Files->size());
        if (cache) {
            size_t readB = Files->read(reinterpret_cast<uint8_t *>(cache), Files->size());

            if (readB != Files->size()) {
                ESP_LOGE("CopyToRam", "Size mismatch %zd != %zd", Files->size(), readB);

                // free(cache[type]);
                // cache[type] = nullptr;
            }
            if (checksum) {
                char crc32[7];
                memcpy(crc32, (const uint8_t *)cache + readB - 10, 6);
                crc32[6] = 0;
                ESP_LOGI("CRC32", "CRC STR: %s", crc32);
                const char *expected = "CRC32:";
                if (strcmp(crc32, expected)) {
                    sysWarn(NString::format(getTranslation(TextKey::RES_WARN_NO_CHECKSUM).c_str(),
                                            crc32, expected));
                    return;
                }
                const uint8_t *crc_bytes = cache + Files->size() - 4; // last 4 bytes
                uint32_t       crc32_val;
                uint32_t       crc32_calc = hw->crc32(0, cache, Files->size() - 10);
                memcpy(&crc32_val, crc_bytes, sizeof(crc32_val));
                ESP_LOGI("CRC32", "PARSED CRC32 is 0x%08X", crc32_val);
                ESP_LOGI("CRC32", "CALCULATED CRC32 is 0x%08X", crc32_calc);
                if (crc32_val != crc32_calc) {
                    sysWarn(
                        NString::format(getTranslation(TextKey::RES_WARN_CHECKSUM_FAILURE).c_str(),
                                        crc32_val, crc32_calc));
                }
            }
        }
    }
}

ResourceSystem res;


