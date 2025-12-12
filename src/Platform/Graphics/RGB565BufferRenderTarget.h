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
        if (buffer) { std::memset(buffer, 0, sz * sizeof(uint16_t)); }

        vp = {0, 0, w, h};
    }

    virtual ~RGB565BufferRenderTarget() {
        delete[] buffer;
        buffer = nullptr;
    }

    void setViewport(Viewport newVp) override {
        // clamp viewport to buffer
        if (newVp.x < 0) { newVp.x = 0; }
        if (newVp.y < 0) { newVp.y = 0; }
        if (newVp.x + newVp.w > width) { newVp.w = width - newVp.x; }
        if (newVp.y + newVp.h > height) { newVp.h = height - newVp.y; }
        vp = newVp;
    }

    Viewport getViewport() override { return vp; }

    void setAddrWindow(uint16_t xs, uint16_t ys, uint16_t w, uint16_t h) override {
        windowX = xs;
        windowY = ys;
        windowW = w;
        windowH = h;


            if (windowX < vp.x) { windowX = vp.x; }
            if (windowY < vp.y) { windowY = vp.y; }
            if (windowX + windowW > vp.x + vp.w) { windowW = (vp.x + vp.w) - windowX; }
            if (windowY + windowH > vp.y + vp.h) { windowH = (vp.y + vp.h) - windowY; }
    }

    void writeColor(uint16_t color, uint32_t len) override {
        color = (color >> 8) | (color << 8);
        if (len > (uint32_t)windowW * windowH) { len = windowW * windowH; }

        for (uint32_t i = 0; i < len; ++i) {
            int16_t px = windowX + (i % windowW);
            int16_t py = windowY + (i / windowW);

            if ((px < vp.x || py < vp.y || px >= vp.x + vp.w || py >= vp.y + vp.h)) { continue; }

            if (px < 0 || py < 0 || px >= width || py >= height) { continue; }
            buffer[py * width + px] = color;
        }
    }
    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if (!buffer) { return; }
        if (x < vp.x || y < vp.y || x >= vp.x + vp.w || y >= vp.y + vp.h) { return; }
        buffer[y * width + x] = color;
    }

    void pushBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *data,
                    bool transparent, uint16_t transpColor) override {
        if (!buffer || !data) { return; }

        for (int ry = 0; ry < h; ++ry) {
            int16_t dstY = y + ry;
            if (dstY < vp.y || dstY >= vp.y + vp.h) { continue; }
            for (int rx = 0; rx < w; ++rx) {
                int16_t dstX = x + rx;
                if (dstX < vp.x || dstX >= vp.x + vp.w) { continue; }
                uint16_t srcPx = data[ry * w + rx];
                if (transparent && srcPx == transpColor) { continue; }
                buffer[dstY * width + dstX] = srcPx;
            }
        }
    }

    void fillScreen(uint16_t color) override {
        if (!buffer) { return; }

        for (int y = vp.y; y < vp.y + vp.h; ++y) {
            for (int x = vp.x; x < vp.x + vp.w; ++x) { buffer[y * width + x] = color; }
        }
    }

  private:
    int16_t  windowX = 0, windowY = 0, windowW = 0, windowH = 0;
    Viewport vp;
};

// Setup rgb565 BufferRenderTarget.
inline RenderTarget *setupBufferRenderTarget(int16_t width, int16_t height) {
    return new RGB565BufferRenderTarget(width, height);
}