// Minimal no-op implementations for TFT_STUB stub declared in Display.h
// This file intentionally avoids Arduino-specific APIs so it can be built
// on desktop/tooling environments. Replace these with platform-specific
// implementations when porting to embedded hardware.
#include "Display.h"
#include <Platform/Graphics/Fonts/FontLoader.h>
#include <System/UTF.h>
TFT_STUB::TFT_STUB(int16_t w, int16_t h)
    : _init_w(w), _init_h(h), _w(w), _h(h), _rotation(0), _cursor_x(0), _cursor_y(0),
      _textcolor(0xFFFF) {}

void TFT_STUB::init(uint8_t) { activeRenderTarget->init(); }

void TFT_STUB::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (activeRenderTarget) { activeRenderTarget->drawPixel(x, y, color); }
}

void TFT_STUB::pushColors(uint16_t *data, int16_t len, bool swap) {
    if (!activeRenderTarget) { return; }
    activeRenderTarget->pushColors(data, len, swap);
}
void TFT_STUB::fillScreen(uint16_t color) {
    if (!activeRenderTarget) { return; }
    activeRenderTarget->fillScreen(color);
}

void TFT_STUB::setAddrWindow(uint16_t xs, uint16_t ys, uint16_t w, uint16_t h) {
    Viewport vp = getViewport();
    xs += vp.x;
    ys += vp.y;
    activeRenderTarget->setAddrWindow(xs, ys, w, h);
}

void TFT_STUB::setViewport(int16_t x, int16_t y, int16_t w, int16_t h) {
    activeRenderTarget->setViewport(Viewport(x, y, w, h));
}

void TFT_STUB::resetViewport() { activeRenderTarget->setViewport(Viewport(0, 0, _w, _h)); }

void TFT_STUB::setRotation(uint8_t r) {
    _rotation = r & 3;
    if ((_rotation & 1) == 0) {
        _w = _init_w;
        _h = _init_h;
    }
    else {
        _w = _init_h;
        _h = _init_w;
    }
}

void TFT_STUB::setTextColor(uint16_t color, uint16_t bg, bool opaque) {
    textcolor     = color;
    _textbgcolor  = bg;
    _textbgopaque = opaque;
}

void TFT_STUB::setTextSize(uint8_t size) { textsize = size; }

void TFT_STUB::setTextFont(uint8_t /*font*/) { /* no-op stub */ }

void TFT_STUB::setTextWrap(bool wrap, bool /*clip*/) { textwrap = wrap; }

int TFT_STUB::getCursorX() const { return _cursor_x; }
int TFT_STUB::getCursorY() const { return _cursor_y; }

Viewport TFT_STUB::getViewport() { return activeRenderTarget->getViewport(); };
void     TFT_STUB::setViewport(Viewport vp) { activeRenderTarget->setViewport(vp); }

void TFT_STUB::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    setAddrWindow(x, y, w, h);

    activeRenderTarget->writeColor(color, w * h);
}

void TFT_STUB::pushImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data) {
    if (activeRenderTarget) { activeRenderTarget->pushBuffer(x, y, w, h, data, false, 0); }
}

void TFT_STUB::pushImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data,
                         uint16_t transpColor) {
    if (activeRenderTarget) {
        // probably should be fixed in ImageEditor
        transpColor = (transpColor >> 8) | (transpColor << 8);
        activeRenderTarget->pushBuffer(x, y, w, h, data, true, transpColor);
    }
}

void TFT_STUB::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    // Top edge
    setAddrWindow(x, y, w, 1);
    activeRenderTarget->writeColor(color, w);
    // Bottom edge
    setAddrWindow(x, y + h - 1, w, 1);
    activeRenderTarget->writeColor(color, w);
    // Left edge
    setAddrWindow(x, y + 1, 1, h - 2);
    activeRenderTarget->writeColor(color, h - 2);
    // Right edge
    setAddrWindow(x + w - 1, y + 1, 1, h - 2);
    activeRenderTarget->writeColor(color, h - 2);
}
void TFT_STUB::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (!activeRenderTarget) { return; }
    if (y < 0 || y >= activeRenderTarget->getHeight()) { return; }

    int16_t startX = x;
    int16_t len    = w;

    // Clip left
    if (startX < 0) {
        len += startX; // reduce length
        startX = 0;
    }

    // Clip right
    if (startX + len > activeRenderTarget->getWidth()) {
        len = activeRenderTarget->getWidth() - startX;
    }

    if (len <= 0) { return; }

    // Use render target's fast write
    setAddrWindow(startX, y, len, 1);
    activeRenderTarget->writeColor(color, len);
}

void TFT_STUB::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy, e2;

    while (true) {
        drawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) { break; }
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void TFT_STUB::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                            uint16_t color) {
    color = (color >> 8) | (color << 8); // swap byte order
    // Sort vertices by y-coordinate ascending (y0 <= y1 <= y2)
    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }
    if (y1 > y2) {
        std::swap(y1, y2);
        std::swap(x1, x2);
    }
    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }

    int16_t totalHeight = y2 - y0;
    for (int16_t i = 0; i < totalHeight; i++) {
        bool    secondHalf    = i > (y1 - y0) || y1 == y0;
        int16_t segmentHeight = secondHalf ? y2 - y1 : y1 - y0;

        float alpha = (float)i / totalHeight;
        float beta =
            (float)(i - (secondHalf ? y1 - y0 : 0)) / segmentHeight; // relative to segment

        int16_t ax = x0 + (x2 - x0) * alpha;
        int16_t bx = secondHalf ? x1 + (x2 - x1) * beta : x0 + (x1 - x0) * beta;

        if (ax > bx) { std::swap(ax, bx); }
        for (int16_t j = ax; j <= bx; j++) { drawPixel(j, y0 + i, color); }
    }
}

fontFile_t TFT_STUB::getFont(uint32_t c, font_t font_) const {
    if (font_.isGFXFontSet && font_.font_set && font_.font_set_count) {
        for (int i = 0; i < font_.font_set_count; i++) {
            if (font_.font_set[i].isFile) {
                FontHolder *fh = getFontHolder(font_.font_set[i].path);
                if (!fh) { continue; }
                for (int j = 0; j < fh->font.size(); j++) {
                    const GFXfontPacked &fnt = fh->font[j];

                    if (c >= fnt.first && c <= fnt.last) {
                //        ESP_LOGI("DISPLAY", "Found glyph 0x%X in font %s", c,
                //                 font_.font_set[i].path.c_str());
                        return font_.font_set[i];
                    }
                }
            }
            else if (font_.font_set[i].gfont) {
                const GFXfont *fnt = font_.font_set[i].gfont;
                if (c >= fnt->first && c <= fnt->last) {
                   // ESP_LOGI("DISPLAY", "Found glyph 0x%X in font", c);
                    return font_.font_set[i];
                }
            }
        }
    }
    return {0, 0, NString()};
}

void TFT_STUB::drawGlyphCore(uint8_t *bitmap, uint32_t bo, uint8_t w, uint8_t h, int16_t xo,
                             int16_t yo, uint16_t xAdvance, uint16_t yAdvance, int16_t x,
                             int16_t y) {
    if (_textbgopaque) {
        uint16_t cellW = xAdvance * textsize;
        uint16_t cellH = yAdvance * textsize;
        fillRect(x, y - cellH, cellW, cellH, _textbgcolor);
    }

    uint16_t hpc  = 0;
    uint8_t  bits = 0, bit = 0;

    for (uint8_t yy = 0; yy < h; yy++) {
        for (uint8_t xx = 0; xx < w; xx++) {

            if (bit == 0) {
                bits = bitmap[bo++];
                bit  = 0x80;
            }

            if (bits & bit) { hpc++; }
            else if (hpc) {
                int drawX = x + (xo + xx - hpc);
                int drawY = y + (yo + yy);

                if (textsize == 1) { drawFastHLine(drawX, drawY, hpc, textcolor); }
                else { fillRect(drawX, drawY, hpc * textsize, textsize, textcolor); }

                hpc = 0;
            }

            bit >>= 1;
        }

        if (hpc) {
            int drawX = x + (xo + w - hpc);
            int drawY = y + (yo + yy);

            if (textsize == 1) { drawFastHLine(drawX, drawY, hpc, textcolor); }
            else { fillRect(drawX, drawY, hpc * textsize, textsize, textcolor); }

            hpc = 0;
        }
    }

    _cursor_x += xAdvance * textsize;
}

void TFT_STUB::renderGlyph(uint32_t c, int16_t x, int16_t y) {
    if (!activeRenderTarget) { return; }

    // Classic 5x7 font
    if (!currentFont.isGFX && !currentFont.font_file.isFile) {
        if (c > 127) { c = '?'; }

        if (_textbgopaque) { fillRect(x, y, 6 * textsize, 8 * textsize, _textbgcolor); }

        for (int col = 0; col < 5; col++) {
            uint8_t line = font[c * 5 + col];
            for (int row = 0; row < 8; row++) {
                if (line & (1 << row)) {
                    for (int sx = 0; sx < textsize; sx++) {
                        for (int sy = 0; sy < textsize; sy++) {
                            drawPixel(x + col * textsize + sx, y + row * textsize + sy, textcolor);
                        }
                    }
                }
            }
        }

        _cursor_x += 6 * textsize;
        return;
    }

    fontFile_t fnt = getFont(c, currentFont);
    if (!fnt.path.isEmpty()) {
        ESP_LOGI("DISPLAY", "Rendering glyph 0x%X from file %s", c, fnt.path.c_str());
        LoadedGlyph *lg = getGlyphData(fnt.path, c);
        if (!lg) {
            ESP_LOGW("DISPLAY", "Failed glyph 0x%X", c);
            return;
        }

        const GFXglyphPacked &g = lg->glyphData;

        uint16_t yAdvance = lg->fontHolder->font[0].yAdvance;

        drawGlyphCore(lg->bitmapData, 0, g.width, g.height, g.xOffset, g.yOffset, g.xAdvance,
                      yAdvance, x, y);
        return;
    }

    if (!currentFont.font_file.gfont && !currentFont.font_set) { return; }

    //if (currentFont.font_file.gfont) { fnt = currentFont.font_file; }

    if (!fnt.gfont) { return; }
    if (c < fnt.gfont->first || c > fnt.gfont->last) {
        ESP_LOGW("FONT", "UNKNOWN CHAR 0x%04X; f: 0x%04X l: 0x%04X", c, fnt.gfont->first,
                 fnt.gfont->last);
        c = '?';
    }

    uint16_t ci = c - fnt.gfont->first;

    GFXglyph *glyph  = &fnt.gfont->glyph[ci];
    uint8_t  *bitmap = fnt.gfont->bitmap;

    if (!glyph || !bitmap) { return; }

    drawGlyphCore(bitmap, glyph->bitmapOffset, glyph->width, glyph->height, glyph->xOffset,
                  glyph->yOffset, glyph->xAdvance, fnt.gfont->yAdvance, x, y);
}

int TFT_STUB::textWidth(const std::string &s) const {
    if (!currentFont.isGFX || !currentFont.font_file.gfont) {
        return static_cast<int>(s.length() * 6 * textsize);
    }

    int w = 0;

    const char *p = s.c_str();
    uint32_t    cp;

    while (*p) {
        const char      *next = utf8_decode(p, &cp);
        const fontFile_t ff   = getFont(cp, currentFont);
        if (!ff.gfont && !ff.isFile) {
            return w + 6 * textsize; // fallback to single char width if font lookup fails
        }
        if (ff.isFile) {
            LoadedGlyph *lg = getGlyphData(ff.path, cp);
            if (lg && lg->fontHolder) {
                GFXglyphPacked &g = lg->glyphData;
                w += g.xAdvance * textsize;
            }
        }
        else if (ff.gfont) {
            if (cp >= ff.gfont->first && cp <= ff.gfont->last) {
                w += ff.gfont->glyph[cp - ff.gfont->first].xAdvance * textsize;
            }
        }
        p = next;
    }

    return w;
}
int TFT_STUB::fontHeight() const {
    if (!currentFont.isGFX || !currentFont.font_file.gfont) {
        // Classic 5x7 font
        return 8 * textsize;
    }
    else if (currentFont.isGFX) {
        // GFX font
        return currentFont.font_file.gfont->yAdvance * textsize;
    }
    else if (currentFont.font_set) {
        // GFX font set - use max yAdvance among fonts in set
        int maxYAdvance = 0;
        for (int i = 0; i < currentFont.font_set_count; i++) {
            const GFXfont *fnt = currentFont.font_set[i].gfont;
            if (fnt && fnt->yAdvance > maxYAdvance) { maxYAdvance = fnt->yAdvance; }
        }
        return maxYAdvance * textsize;
    }
    else if (currentFont.font_file.isFile) {
        return getFontYAdvance(currentFont.font_file.path) * textsize;
    }
    return 0; // unknown font type
}

void TFT_STUB::setAttribute(int /*attr*/, bool /*value*/) { /* no-op */ }

uint8_t TFT_STUB::getRotation() const { return _rotation; }

int16_t TFT_STUB::width() const { return _w; }
int16_t TFT_STUB::height() const { return _h; }

uint16_t TFT_STUB::color565(uint8_t red, uint8_t green, uint8_t blue) const {
    uint16_t color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);

    return (color << 8) | (color >> 8);
}

void TFT_STUB::setCursor(int16_t x, int16_t y) {
    _cursor_x = x;
    _cursor_y = y;
}

void TFT_STUB::printf(const char *fmt, ...) {
    char    buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    print(buf);
}

void TFT_STUB::print(const char *str) {
    if (!str) { return; }
    const char *p = str;
    while (*p) {
        uint32_t uc;
        p = utf8_decode(p, &uc);
        print(uc);
    }
}
void TFT_STUB::print(uint32_t c) {
    if (c == '\n') {
        _cursor_x = 0;
        _cursor_y += fontHeight();
    }
    else if (c == '\r') { _cursor_x = 0; }
    else {
        if (textwrap && (_cursor_x > (width() - textWidth("W")))) {
            _cursor_x = 0;
            _cursor_y += fontHeight();
        }
        renderGlyph(c, _cursor_x, _cursor_y);
    }
}

void TFT_STUB::println(const char *str) {
    print(str);
    print('\n');
}

void TFT_STUB::println(uint32_t c) {
    print(c);
    print('\n');
}

void TFT_STUB::print(const NString &s) { print(s.c_str()); }

void TFT_STUB::println(const NString &s) {
    print(s);
    print("\n");
}

void TFT_STUB::setRenderTarget(RenderTarget *target) { activeRenderTarget = target; }
