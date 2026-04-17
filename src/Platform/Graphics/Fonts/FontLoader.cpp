#include "FontLoader.h"
const char*              TAG        = "FontLoader";
const char*              headerName = "ADAFONT";
std::vector<FontHolder>  loadedFonts;
std::deque<LoadedGlyph*> loadedGlyphs;
size_t                   MAX_GLYPH_CACHE_SIZE = 128;

void loadFont(const NString& path) {
    std::vector<GFXfontPacked> fonts;
    fontsHeader                header;
    NFile*                     file = VFS.open(path, "rb");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open font file: %s", path.c_str());
        return;
    }
    if (file->read(&header, sizeof(header)) != sizeof(header)) {
        ESP_LOGE(TAG, "Failed to read font header");

        return;
    }
    if (strncmp(header.headerName, headerName, strlen(headerName)) != 0) {
        ESP_LOGE(TAG, "Invalid font file: %s", path.c_str());

        return;
    }

    for (int i = 0; i < header.count; i++) {
        GFXfontPacked font;
        if (file->read(&font, sizeof(font)) != sizeof(font)) {
            ESP_LOGE(TAG, "Failed to read font data for font %d", i);
            return;
        }
        file->seek(file->position() + font.bitmapSize +
                       sizeof(GFXglyphPacked) * (font.last - font.first + 1),
                   SEEK_SET);

        fonts.push_back(font);
        ESP_LOGI(TAG, "Loaded font %d: first=0x%04X last=0x%04X yAdvance=%d bitmapSize=%d", i,
                 font.first, font.last, font.yAdvance, font.bitmapSize);
    }
    loadedFonts.push_back({path, file, fonts});
}

void unloadFont(const NString& path) {
    auto it = std::remove_if(loadedFonts.begin(), loadedFonts.end(),
                             [&path](const FontHolder& fh) { return fh.path == path; });
    if (it != loadedFonts.end()) { loadedFonts.erase(it, loadedFonts.end()); }

    for (LoadedGlyph* lg : loadedGlyphs) {
        if (!lg || !lg->fontHolder) { continue; }
        if (lg->fontHolder->path == path) {
            free(lg->bitmapData);
            free(lg);
        }
    }

    loadedGlyphs.erase(std::remove_if(loadedGlyphs.begin(), loadedGlyphs.end(),
                                      [&path](LoadedGlyph* lg) {
                                          if (!lg || !lg->fontHolder) { return false; }
                                          return lg->fontHolder->path == path;
                                      }),
                       loadedGlyphs.end());
}

LoadedGlyph* getGlyphData(const NString& path, uint16_t charCode) {
    for (LoadedGlyph* lg : loadedGlyphs) {
        if (!lg) { continue; }
        if (!lg->fontHolder) { continue; }
        if (lg->fontHolder->path == path && lg->charCode == charCode) { return lg; }
    }
    FontHolder* holder = nullptr;
    for (FontHolder& fholder : loadedFonts) {
        if (fholder.path == path) {
            holder = &fholder;
            break;
        }
    }
    if (!holder) {
        ESP_LOGE(TAG, "Font not loaded: %s", path.c_str());
        return nullptr;
    }
    if (!holder->file->available()) {
        ESP_LOGE(TAG, "Font file not available: %s", path.c_str());
        holder->file = VFS.open(path, "rb");
    }

    for (size_t i = 0; i < holder->font.size(); i++) {
        if (charCode >= holder->font[i].first && charCode <= holder->font[i].last) {
            GFXfontPacked& fnt = holder->font[i];
            size_t         pos = 0;
            pos += sizeof(fontsHeader);
            for (size_t j = 0; j < i; j++) {
                pos += sizeof(GFXfontPacked);
                pos += holder->font[j].bitmapSize;
                pos += (holder->font[j].last - holder->font[j].first + 1) * sizeof(GFXglyphPacked);
            }
            pos += sizeof(GFXfontPacked);
            size_t bitmapDataOffset = pos + sizeof(GFXglyphPacked) * (fnt.last - fnt.first + 1);
            pos += (charCode - fnt.first) * sizeof(GFXglyphPacked);

            if (!holder->file->seek(pos, SEEK_SET)) {
                ESP_LOGE(TAG, "Failed to seek to glyph data for char code 0x%04X in font %s",
                         charCode, path.c_str());
                return nullptr;
            }
            GFXglyphPacked glyphData;
            if (holder->file->read(&glyphData, sizeof(glyphData)) != sizeof(glyphData)) {
                ESP_LOGE(TAG, "Failed to read glyph data for char code 0x%04X in font %s",
                         charCode, path.c_str());
                return nullptr;
            }
            size_t posBeforeRead = holder->file->position();

            size_t   bytesToRead = (glyphData.width * glyphData.height + 7) / 8;
            uint8_t* bitmapData  = (uint8_t*)ps_malloc(bytesToRead);
            if (!holder->file->seek(bitmapDataOffset + glyphData.bitmapOffset, SEEK_SET)) {
                ESP_LOGE(TAG, "Failed to seek to bitmap data for char code 0x%04X in font %s",
                         charCode, path.c_str());
                free(bitmapData);
                return nullptr;
            }
            size_t bytesRead = holder->file->read(bitmapData, bytesToRead);
            if (bytesRead != bytesToRead) {
                ESP_LOGE(TAG,
                         "Failed to read bitmap data for char code 0x%04X in font %s"
                         "(expected %zd bytes, got %zd bytes) pos=%zd, fileSize=%zd posBeforeRead=%zd",
                         charCode, path.c_str(), bytesToRead, bytesRead, holder->file->position(),
                         holder->file->size(), posBeforeRead);
                free(bitmapData);
                return nullptr;
            }

            LoadedGlyph* loadedGlyph = (LoadedGlyph*)ps_malloc(sizeof(LoadedGlyph));
            loadedGlyph->charCode    = charCode;
            loadedGlyph->glyphData   = glyphData;
            loadedGlyph->bitmapData  = bitmapData;
            loadedGlyph->fontHolder  = holder;
            loadedGlyphs.push_back(loadedGlyph);
            if (loadedGlyphs.size() >= MAX_GLYPH_CACHE_SIZE) {
                LoadedGlyph* lg = loadedGlyphs.front();
                loadedGlyphs.pop_front();
                if (lg) {
                    ESP_LOGD(TAG, "Evicting glyph 0x%04X", lg->charCode);
                    free(lg->bitmapData);
                    free(lg);
                }
            }
            return loadedGlyph;
        }
    }
    return nullptr;
}

int getFontYAdvance(const NString& path) {
    for (FontHolder& fholder : loadedFonts) {
        if (fholder.path == path) {
            if (!fholder.font.empty()) {
                int yAdvance = 0;
                for (GFXfontPacked& font : fholder.font) {
                    if (yAdvance < font.yAdvance) { yAdvance = font.yAdvance; }
                }
                return yAdvance;
            }
        }
    }
    return 0;
}

FontHolder* getFontHolder(const NString& path) {
    for (FontHolder& fholder : loadedFonts) {
        if (fholder.path == path) { return &fholder; }
    }
    return nullptr;
}