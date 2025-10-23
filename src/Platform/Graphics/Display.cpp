// Minimal no-op implementations for TFT_STUB stub declared in Display.h
// This file intentionally avoids Arduino-specific APIs so it can be built
// on desktop/tooling environments. Replace these with platform-specific
// implementations when porting to embedded hardware.
#include "Display.h"

TFT_STUB::TFT_STUB(int16_t w, int16_t h)
    : _init_w(w), _init_h(h), _w(w), _h(h), _rotation(0), _cursor_x(0), _cursor_y(0),
      _textcolor(0xFFFF), _vp_x(0), _vp_y(0), _vp_w(w), _vp_h(h), _vp_active(false) {}

void TFT_STUB::init(uint8_t) {}

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
void TFT_STUB::pushColors(const uint16_t *data, int16_t len, bool swap) {
    if (activeRenderTarget) {
        activeRenderTarget->pushBuffer(_addrX1, _addrY1, _addrX2, _addrY2, data, false, 0);
        activeRenderTarget->present();
    }
}
void TFT_STUB::fillScreen(uint16_t color) {
    if (!_vp_active) {
        // fill entire screen (no-op implementation)
        (void)color;
        return;
    }
    // fill viewport region (no-op): the hardware implementation should
    // write only the rectangle [_vp_x,_vp_y]..[_vp_x+_vp_w-1,_vp_y+_vp_h-1]
    (void)color;
}

void TFT_STUB::setAddrWindow(uint16_t xs, uint16_t ys, uint16_t w, uint16_t h) {
    if (_vp_active) {
        int32_t abs_xs = (int32_t)xs + _vp_x;
        int32_t abs_ys = (int32_t)ys + _vp_y;
        int32_t abs_x2 = abs_xs + w;
        int32_t abs_y2 = abs_ys + h;

        int32_t vx1 = _vp_x;
        int32_t vy1 = _vp_y;
        int32_t vx2 = _vp_x + _vp_w;
        int32_t vy2 = _vp_y + _vp_h;

        int32_t ix1 = abs_xs > vx1 ? abs_xs : vx1;
        int32_t iy1 = abs_ys > vy1 ? abs_ys : vy1;
        int32_t ix2 = abs_x2 < vx2 ? abs_x2 : vx2;
        int32_t iy2 = abs_y2 < vy2 ? abs_y2 : vy2;

        if (ix2 <= ix1 || iy2 <= iy1) {
            // empty intersection, nothing to set
            return;
        }
        setViewport((int16_t)ix1, (int16_t)iy1, (int16_t)(ix2 - ix1), (int16_t)(iy2 - iy1));
        return;
    }
    _addrX1 = xs;
    _addrY1 = ys;
    _addrX2 = w;
    _addrY2 = h;
    (void)xs;
    (void)ys;
    (void)w;
    (void)h;
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

void TFT_STUB::setTextColor(uint16_t color, uint16_t bg, bool opaque) { textcolor = color; }

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
    for (int16_t i = x; i < x + w; i++) {
        for (int16_t j = y; j < y + h; j++) { drawPixel(i, j, color); }
    }
}

void TFT_STUB::pushImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data) {
    // translate relative coords when viewport active
    if (_vp_active) {
        x += _vp_x;
        y += _vp_y;
    }
    setAddrWindow(x, y, w, h);
    pushColors(data, w * h);
    activeRenderTarget->present();
}

void TFT_STUB::pushImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data,
                         uint16_t transpColor) {
    // translate relative coords when viewport active
    if (_vp_active) {
        x += _vp_x;
        y += _vp_y;
    }
    setAddrWindow(x, y, w, h);
    pushColors(data, w * h);
    activeRenderTarget->present();
}

void TFT_STUB::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (_vp_active) {
        x += _vp_x;
        y += _vp_y;
    }
    for (int16_t i = x; i < x + w; i++) {
        drawPixel(i, y, color);
        drawPixel(i, y + h - 1, color);
    }
    for (int16_t j = y; j < y + h; j++) {
        drawPixel(x, j, color);
        drawPixel(x + w - 1, j, color);
    }
    activeRenderTarget->present();
}

void TFT_STUB::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (_vp_active) {
        x0 += _vp_x;
        y0 += _vp_y;
        x1 += _vp_x;
        y1 += _vp_y;
    }
    (void)x0;
    (void)y0;
    (void)x1;
    (void)y1;
    (void)color;
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
    (void)x0;
    (void)y0;
    (void)x1;
    (void)y1;
    (void)x2;
    (void)y2;
    (void)color;
}

void TFT_STUB::renderGlyph(char c, int16_t x, int16_t y) {

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
    // Each character is 5x7, stored in 5 bytes, each byte represents a vertical column
    for (int col = 0; col < 5; col++) {
        uint8_t line = font[c * 5 + col];
        // Read bits from bottom to top (bit 0 is top pixel)
        for (int row = 0; row < 8; row++) {
            if (line & (1 << row)) { // Start from top bit (bit 6)
                // Draw a textsize x textsize pixel block
                for (int sx = 0; sx < textsize; sx++) {
                    for (int sy = 0; sy < textsize; sy++) {
                        drawPixel(x + col * textsize + sx, y + row * textsize + sy, textcolor);
                    }
                }
            }
        }
    }
    // Advance cursor after rendering
    _cursor_x += 6 * textsize; // Character width (5) + 1 pixel space between chars
    activeRenderTarget->present();
}

int TFT_STUB::textWidth(const std::string &s) const {
    return s.length() * 6 * textsize; // 5 pixels width + 1 pixel spacing
}

int TFT_STUB::fontHeight() const {
    return 8 * textsize; // Character height
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

void TFT_STUB::setRenderTarget(RenderTarget *target) {
    // If there is an existing render target, call its deinit hook
    if (activeRenderTarget && activeRenderTarget != target) { activeRenderTarget->deinit(); }

    activeRenderTarget = target;

    // Initialize the new render target if present
    if (activeRenderTarget) { activeRenderTarget->init(); }
}
