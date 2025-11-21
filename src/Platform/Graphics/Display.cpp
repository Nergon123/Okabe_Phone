// Minimal no-op implementations for TFT_STUB stub declared in Display.h
// This file intentionally avoids Arduino-specific APIs so it can be built
// on desktop/tooling environments. Replace these with platform-specific
// implementations when porting to embedded hardware.
#include "Display.h"

TFT_STUB::TFT_STUB(int16_t w, int16_t h)
    : _init_w(w), _init_h(h), _w(w), _h(h), _rotation(0), _cursor_x(0), _cursor_y(0),
      _textcolor(0xFFFF), _vp_x(0), _vp_y(0), _vp_w(w), _vp_h(h), _vp_active(false) {}

void TFT_STUB::init(uint8_t) { activeRenderTarget->init(); }

void TFT_STUB::drawPixel(int16_t x, int16_t y, uint16_t color) {
    // If viewport active, input coordinates are relative to the viewport origin.
    if (_vp_active) {
        // translate to absolute display coords
        int32_t ax = (int32_t)x + _vp_x;
        int32_t ay = (int32_t)y + _vp_y;
        // clip to viewport
        if (ax < _vp_x || ay < _vp_y) { return; }
        if (ax >= _vp_x + _vp_w || ay >= _vp_y + _vp_h) { return; }
        x = (int16_t)ax;
        y = (int16_t)ay;
    }
    else {
        // clip to display bounds
        if (x < 0 || y < 0 || x >= _w || y >= _h) { return; }
    }
    if (activeRenderTarget) { activeRenderTarget->drawPixel(x, y, color); }
}

void TFT_STUB::pushColors(uint16_t *data, int16_t len, bool swap) {
    if (!activeRenderTarget) { return; }
    activeRenderTarget->pushColors(data, len, swap);
}
void TFT_STUB::fillScreen(uint16_t color) {
    if (!activeRenderTarget) { return; }
    if (!_vp_active) {
        // fill entire screen (no-op implementation)
        activeRenderTarget->fillScreen(color);
        return;
    }
    fillRect(_vp_x, _vp_y, _vp_w, _vp_h, color);
}

void TFT_STUB::setAddrWindow(uint16_t xs, uint16_t ys, uint16_t w, uint16_t h) {
    // if (_vp_active) {
    //     int32_t abs_xs = (int32_t)xs + _vp_x;
    //     int32_t abs_ys = (int32_t)ys + _vp_y;
    //     int32_t abs_x2 = abs_xs + w;
    //     int32_t abs_y2 = abs_ys + h;

    //     int32_t vx1 = _vp_x;
    //     int32_t vy1 = _vp_y;
    //     int32_t vx2 = _vp_x + _vp_w;
    //     int32_t vy2 = _vp_y + _vp_h;

    //     int32_t ix1 = abs_xs > vx1 ? abs_xs : vx1;
    //     int32_t iy1 = abs_ys > vy1 ? abs_ys : vy1;
    //     int32_t ix2 = abs_x2 < vx2 ? abs_x2 : vx2;
    //     int32_t iy2 = abs_y2 < vy2 ? abs_y2 : vy2;

    //     if (ix2 <= ix1 || iy2 <= iy1) {
    //         // empty intersection, nothing to set
    //         return;
    //     }
    //     //setViewport((int16_t)ix1, (int16_t)iy1, (int16_t)(ix2 - ix1), (int16_t)(iy2 - iy1));
    //     return;
    // }
    activeRenderTarget->setAddrWindow(xs, ys, w, h);
}

void TFT_STUB::setViewport(int16_t x, int16_t y, int16_t w, int16_t h) {
    // clamp to display bounds
    if (x < 0) { x = 0; }
    if (y < 0) { y = 0; }
    if (w < 0) { w = 0; }
    if (h < 0) { h = 0; }
    if (x + w > _init_w) { w = _init_w - x; }
    if (y + h > _init_h) { h = _init_h - y; }
    _vp_x      = x;
    _vp_y      = y;
    _vp_w      = w;
    _vp_h      = h;
    _vp_active = (w > 0 && h > 0);
}

void TFT_STUB::resetViewport() {
    _vp_x      = 0;
    _vp_y      = 0;
    _vp_w      = _init_w;
    _vp_h      = _init_h;
    _vp_active = false;
}

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

int  TFT_STUB::getViewportWidth() const { return _vp_w; }
int  TFT_STUB::getViewportHeight() const { return _vp_h; }
int  TFT_STUB::getViewportX() const { return _vp_x; }
int  TFT_STUB::getViewportY() const { return _vp_y; }
bool TFT_STUB::getViewportDatum() const { return _vp_datum; }

void TFT_STUB::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    // translate relative coords when viewport active
    if (_vp_active) {
        x += _vp_x;
        y += _vp_y;
    }
    setAddrWindow(x, y, w, h);

    activeRenderTarget->writeColor(color, w * h);
    activeRenderTarget->present();
}

void TFT_STUB::pushImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data) {
    // translate relative coords when viewport active
    if (_vp_active) {
        x += _vp_x;
        y += _vp_y;
    }
    if (activeRenderTarget) {
        activeRenderTarget->pushBuffer(x, y, w, h, data, false, 0);
        activeRenderTarget->present();
    }
}

void TFT_STUB::pushImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data,
                         uint16_t transpColor) {
    // translate relative coords when viewport active
    if (_vp_active) {
        x += _vp_x;
        y += _vp_y;
    }
    if (activeRenderTarget) {
        activeRenderTarget->pushBuffer(x, y, w, h, data, true, transpColor);
        activeRenderTarget->present();
    }
}

void TFT_STUB::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (_vp_active) {
        x += _vp_x;
        y += _vp_y;
    }
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

    activeRenderTarget->present();
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
    activeRenderTarget->setAddrWindow(startX, y, len, 1);
    activeRenderTarget->writeColor(color, len);
}

void TFT_STUB::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (_vp_active) {
        x0 += _vp_x;
        y0 += _vp_y;
        x1 += _vp_x;
        y1 += _vp_y;
    }
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

    activeRenderTarget->present();
}

void TFT_STUB::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                            uint16_t color) {
    if (_vp_active) {
        x0 += _vp_x;
        y0 += _vp_y;
        x1 += _vp_x;
        y1 += _vp_y;
        x2 += _vp_x;
        y2 += _vp_y;
    }

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

    activeRenderTarget->present();
}

void TFT_STUB::renderGlyph(char c, int16_t x, int16_t y) {
    // Translate coords if viewport active
    if (_vp_active) {
        int32_t ax = (int32_t)x + _vp_x;
        int32_t ay = (int32_t)y + _vp_y;
        if (ax < _vp_x || ay < _vp_y) { return; }
        if (ax >= _vp_x + _vp_w || ay >= _vp_y + _vp_h) { return; }
        x = (int16_t)ax;
        y = (int16_t)ay;
    }
    else {
        if (x < 0 || y < 0 || x >= _w || y >= _h) { return; }
    }

    if (!currentFont.isGFX) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc > 127) {
            return; // or the max index of your font array
        }
        if (_textbgopaque) { fillRect(x, y, 5 * textsize, 8 * textsize, _textbgcolor); }

        for (int col = 0; col < 5; col++) {
            uint8_t line = font[uc * 5 + col];
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
    }
    else if (currentFont.font) {
        c -= currentFont.font->first;                        // character index
        GFXglyph *glyph  = &currentFont.font->glyph[(int)c]; // correct glyph for this char
        uint8_t  *bitmap = currentFont.font->bitmap;
        uint32_t  bo     = glyph->bitmapOffset;
        uint8_t   w = glyph->width, h = glyph->height;
        int8_t    xo = glyph->xOffset, yo = glyph->yOffset;
        uint8_t   xx, yy, bits = 0, bit = 0;
        int16_t   xo16 = 0, yo16 = 0;
        if (_textbgopaque) {
            uint16_t cellW = glyph->xAdvance * textsize;
            uint16_t cellH = currentFont.font->yAdvance * textsize;
            fillRect(x, y - cellH, cellW, cellH, _textbgcolor);
        }

        if (textsize > 1) {
            xo16 = xo;
            yo16 = yo;
        }

        // GFXFF rendering speed up
        uint16_t hpc = 0; // Horizontal foreground pixel count
        for (yy = 0; yy < h; yy++) {
            for (xx = 0; xx < w; xx++) {
                if (bit == 0) {
                    bits = bitmap[bo++];
                    bit  = 0x80;
                }
                if (bits & bit) { hpc++; }
                else {
                    if (hpc) {
                        if (textsize == 1) {
                            drawFastHLine(x + xo + xx - hpc, y + yo + yy, hpc, textcolor);
                        }
                        else {
                            fillRect(x + (xo16 + xx - hpc) * textsize, y + (yo16 + yy) * textsize,
                                     textsize * hpc, textsize, textcolor);
                        }
                        hpc = 0;
                    }
                }
                bit >>= 1;
            }
            // Draw pixels for this line as we are about to increment yy
            if (hpc) {
                if (textsize == 1) {
                    drawFastHLine(x + xo + xx - hpc, y + yo + yy, hpc, textcolor);
                }
                else {
                    fillRect(x + (xo16 + xx - hpc) * textsize, y + (yo16 + yy) * textsize,
                             textsize * hpc, textsize, textcolor);
                }
                hpc = 0;
            }
        }
        _cursor_x += glyph->xAdvance * textsize;
    }
}

int TFT_STUB::textWidth(const std::string &s) const {
    if (!currentFont.isGFX || !currentFont.font) {
        // Classic 5x7 font
        return static_cast<int>(s.length() * 6 * textsize); // 5 pixels + 1 spacing
    }
    else {
        // GFX font
        int            w   = 0;
        const GFXfont *gfx = currentFont.font;
        for (char c : s) {
            if (c < gfx->first || c > gfx->last) {
                continue; // skip missing chars
            }
            GFXglyph *glyph = &gfx->glyph[c - gfx->first];
            w += glyph->xAdvance * textsize;
        }
        return w;
    }
}

int TFT_STUB::fontHeight() const {
    if (!currentFont.isGFX || !currentFont.font) {
        // Classic 5x7 font
        return 8 * textsize;
    }
    else {
        // GFX font
        return currentFont.font->yAdvance * textsize;
    }
}

void TFT_STUB::setAttribute(int /*attr*/, bool /*value*/) { /* no-op */ }

uint8_t TFT_STUB::getRotation() const { return _rotation; }

int16_t TFT_STUB::width() const { return _w; }
int16_t TFT_STUB::height() const { return _h; }

uint16_t TFT_STUB::color565(uint8_t red, uint8_t green, uint8_t blue) const {
    return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
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
    while (*str) {
        if (*str == '\n') {
            _cursor_x = 0;
            _cursor_y += fontHeight();
        }
        else if (*str == '\r') { _cursor_x = 0; }
        else {
            if (textwrap && (_cursor_x >= width())) {
                _cursor_x = 0;
                _cursor_y += fontHeight();
            }
            renderGlyph(*str, _cursor_x, _cursor_y);
        }
        str++;
    }
    activeRenderTarget->present();
}

void TFT_STUB::println(const char *str) {
    print(str);
    print("\n");
}

void TFT_STUB::print(char c) {
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

void TFT_STUB::println(char c) {
    print(c);
    print("\n");
}

void TFT_STUB::print(const NString &s) { this->print(s.c_str()); }

void TFT_STUB::println(const NString &s) {
    print(s);
    print("\n");
}

void TFT_STUB::setRenderTarget(RenderTarget *target) { activeRenderTarget = target; }
