#include "ResourceSystem.h"
#include "Generic.h"
Coords czero = {0, 0};
Coords cnone = {-1, -1};
void   ResourceSystem::Init(File Main, File Wallpapers) {
    Files[RES_MAIN]       = Main;
    Files[RES_WALLPAPERS] = Wallpapers;
    for (int i = 0; i < sizeof(Files) / sizeof(Files[0]); i++) {
        if (!Files[i].available()) {
            ESP_LOGW("RES", "%s resource file is not available. Using Built-in.", names[i]);
        } else {
            parseResourceFile(Files[i], Headers[i], i, i == RES_MAIN);
        }
    }
};

void ResourceSystem::parseResourceFile(File file, Header &header, uint8_t type, bool important) {
    Images[type].clear();
    if (file.read(reinterpret_cast<uint8_t *>(&header), sizeof(Header)) != sizeof(Header)) {
        failure("Error when reading Main Header", important);
        return;
    };
    if (strcmp(header.MAGIC, "NerPh") != 0) {
        failure("Header Mismatch (Wrong file?)", important);
        file = File();
        return;
    }
    if (header.version != FILE_VERSION) {
        failure("File version mismatch.", important);
        file = File();
        return;
    }
    if (header.imageCount == 0) {
        failure("There is no Images...", important);
        file = File();
        return;
    }
    for (int i = 0; i < header.imageCount; ++i) {
        ImageData img;
        if (file.read(reinterpret_cast<uint8_t *>(&img), sizeof(ImageData)) != sizeof(ImageData)) {
            failure("Error reading ImageData", important);
            return;
        }
        ESP_LOGV("RES", "Parsed image %d: ID: %d, Count: %d, X: %d, Y: %d, Width: %d, Height: %d, Offset: %d\n", i, img.id, img.count, img.x, img.y, img.width, img.height, img.offset);
        Images[type].push_back(img);
    }
}

void ResourceSystem::failure(const char *msg, bool important) {
    ESP_LOGE("RES", "%s", msg);
    if (important)
        sysError(msg);
}

ImageData ResourceSystem::GetImageDataByID(uint16_t id, uint8_t type) {
    for (ImageData img : Images[type]) {
        if (img.id == id) {
            return img;
        }
    }
    return ImageData(R_NULL_IMAGE);
}

bool ResourceSystem::DrawImage(uint16_t id, uint8_t index, Coords pos, Coords startpos, Coords endpos, uint8_t type, bool is_screen_buffer, TFT_eSprite &sbuffer) {

    ImageData img = GetImageDataByID(id, type);
    if (img.id == R_NULL_IMAGE && id != R_NULL_IMAGE) {
        ESP_LOGE("RES", "Requested Sprite %d:%d not found", id, index);
        return false;
    } else if (id == R_NULL_IMAGE) {
        return false;
    }
    if (pos.x < 0) {
        pos.x = img.x;
    }
    if (pos.y < 0) {
        pos.y = img.y;
    }
    if (endpos.x <= 0) {
        endpos.x = img.width;
    }
    if (endpos.y <= 0) {
        endpos.y = img.height;
    }
    if (startpos.y < 0)
        startpos.y = 0;
    if (startpos.x < 0)
        startpos.x = 0;
    if (endpos.y > img.height)
        endpos.y = img.height;
    if (endpos.x > img.width)
        endpos.x = img.width;

    if (startpos.y >= endpos.y) {
        return false;
    }
    uint32_t start = (startpos.y * img.width * 2 + (img.width * img.height * 2 * index));
    uint32_t size  = (endpos.y - startpos.y) * img.width * 2;
    if (size == 0) {
        return false;
    }

    uint16_t   *imageBuffer = GetRGB565(img, size, start, type);
    TFT_eSprite spr(&tft);
    spr.createSprite(endpos.x - startpos.x, endpos.y - startpos.y);
    spr.pushImage(-startpos.x, 0, img.width, endpos.y - startpos.y, imageBuffer);
    if (img.flags & 1 /*if transparent*/) {
        if (is_screen_buffer) {
            spr.pushToSprite(&sbuffer, pos.x, pos.y, img.transpColor);
        } else {
            spr.pushSprite(pos.x, pos.y, img.transpColor);
        }
    } else {
        if (is_screen_buffer) {
            spr.pushToSprite(&sbuffer, pos.x, pos.y);
        } else {
            spr.pushSprite(pos.x, pos.y);
        }
    }
    spr.deleteSprite();
    delete[] imageBuffer;
    return true;
}
bool ResourceSystem::DrawImage(uint16_t id, uint8_t index, bool is_screen_buffer, TFT_eSprite &sbuffer) {
    return DrawImage(id, index, {-1, -1}, {0, 0}, {-1, -1}, RES_MAIN, is_screen_buffer, sbuffer);
}
uint16_t *ResourceSystem::GetRGB565(ImageData img, size_t size, uint32_t start, uint8_t type) {
    uint16_t *buffer = new uint16_t[size / 2]; // safer than using uint8_t*
    Files[type].seek(img.offset + start);
    Files[type].read(reinterpret_cast<uint8_t *>(buffer), size);
    return buffer;
}

ResourceSystem res;

void drawWallpaper() {
    res.DrawImage(R_DEFAULT_WALLPAPER);
}