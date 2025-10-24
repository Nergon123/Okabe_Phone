#pragma once
#include "RenderTargets.h"
#include <cstdint>
#include <cstring>

class RGB565BufferRenderTarget : public RenderTarget {
  public:
    RGB565BufferRenderTarget(int16_t w, int16_t h)
        : RenderTarget(RENDER_TARGET_TYPE_BUFFER, w, h, nullptr) {
        size_t sz = static_cast<size_t>(w) * static_cast<size_t>(h);
        buffer    = (sz > 0) ? new uint16_t[sz] : nullptr;
        // initialize to 0 (black)
        if (buffer) { std::memset(buffer, 0, sz * sizeof(uint16_t)); }
    }

    virtual ~RGB565BufferRenderTarget() {
        delete[] buffer;
        buffer = nullptr;
    }
    void fillScreen(uint16_t color) override {
        if (!buffer) { return; }
        for (int i = 0; i < width * height; ++i) { buffer[i] = color; }
    }

    void setAddrWindow(uint16_t xs, uint16_t ys, uint16_t w, uint16_t h) override {
        windowX = xs;
        windowY = ys;
        windowW = w;
        windowH = h;
    }

    void writeColor(uint16_t color, uint32_t len) override {
        if (len > (uint32_t)windowW * windowH) { len = windowW * windowH; }

        for (uint32_t i = 0; i < len; ++i) {
            int16_t px = windowX + (i % windowW);
            int16_t py = windowY + (i / windowW);
            if (px < 0 || py < 0 || px >= width || py >= height) { continue; }
            buffer[py * width + px] = color;
        }
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if (!buffer) { return; }
        if (x < 0 || y < 0 || x >= width || y >= height) { return; }
        buffer[y * width + x] = color;
    }

    void pushBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data,
                    bool transparent, uint16_t transpColor) override {
        if (!buffer || !data) { return; }
        // clip source rectangle to target buffer
        for (int ry = 0; ry < h; ++ry) {
            int16_t dstY = y + ry;
            if (dstY < 0 || dstY >= height) { continue; }
            for (int rx = 0; rx < w; ++rx) {
                int16_t dstX = x + rx;
                if (dstX < 0 || dstX >= width) { continue; }
                uint16_t srcPx = data[ry * w + rx];
                if (transparent && srcPx == transpColor) { continue; }
                buffer[dstY * width + dstX] = srcPx;
            }
        }
    }

    void    init() override { /* already allocated in ctor */ }
    void    deinit() override { /* cleanup in dtor */ }
    int16_t windowX = 0, windowY = 0, windowW = 0, windowH = 0;
};

// Provide the previously-declared factory here inlined for convenience.
inline RenderTarget *setupBufferRenderTarget(int16_t width, int16_t height) {
    return new RGB565BufferRenderTarget(width, height);
}